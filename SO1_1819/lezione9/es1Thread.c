#include "xerrors.h"
#include <ctype.h>
#define Buf_size 10
#define Max_line_len 100

//struttura dati per buffer produttore consumatore
typedef struct {
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex;
  char (*buff)[Max_line_len];
} pcBuffer;

//struttura dati per thread
typedef struct{
  pcBuffer *pcbuff1;
  pcBuffer *pcbuff2;
  const char *outputFileName;
} threadArg;

// funzione eseguita dal thread t2
void *tbody2(void *arg);

// funzione eseguita dal thread t3
void *tbody3(void *arg);

// scambia maiuscole minuscole ad una stringa
void swapStringCase(char *s, size_t lenght);

int main(int argc, char const *argv[]) {
  //leggi input
  if(argc!=3) {
    printf("Uso\n\t%s infile outfile\n", argv[0]);
    exit(1);
  }

  //init buffer produttore consumatore
  pcBuffer buff_one, buff_two;
  buff_one.sem_free_slots = malloc(sizeof(sem_t));
  buff_one.sem_data_items = malloc(sizeof(sem_t));
  buff_one.mutex = malloc(sizeof(pthread_mutex_t));
  buff_one.buff = malloc(sizeof(char)*Buf_size*Max_line_len);

  buff_two.sem_free_slots = malloc(sizeof(sem_t));
  buff_two.sem_data_items = malloc(sizeof(sem_t));
  buff_two.mutex = malloc(sizeof(pthread_mutex_t));
  buff_two.buff = malloc(sizeof(char)*Buf_size*Max_line_len);

  //init semafori dei buffer
  xsem_init(buff_one.sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(buff_one.sem_data_items, 0, 0,__LINE__,__FILE__);
  xsem_init(buff_two.sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(buff_two.sem_data_items, 0, 0,__LINE__,__FILE__);

  //init mutex dei Buffer
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
  xpthread_mutex_init(buff_one.mutex, &attr, __LINE__, __FILE__);
  xpthread_mutex_init(buff_two.mutex, &attr, __LINE__, __FILE__);

  //facciamo partire t2 e t3
  pthread_t t2, t3;
  threadArg arg;
  arg.pcbuff1 = &buff_one;
  arg.pcbuff2 = &buff_two;
  arg.outputFileName = argv[2];
  xpthread_create(&t2, NULL, tbody2, (void *) &arg, __LINE__, __FILE__);
  xpthread_create(&t3, NULL, tbody3, (void *) &arg, __LINE__, __FILE__);

  //thread t1: leggo una riga da infile (rappresenta l'unità di lavoro) e la
  //inserisco nel buffer buff_one.

  //apertura file
  FILE *f = xfopen(argv[1], "r", __LINE__, __FILE__);
  if(f == NULL){
    perror("Problema con l'apertura del file");
  }

  //predisposizione buffer di lettura
  char *linea = malloc(Max_line_len);
  if(linea == NULL){
    perror("Problema malloc\n");
    exit(1);
  } //scriviamo tutti zeri nel buffer
  if(memset(linea, 0, Max_line_len) == NULL){
    perror("Problema con memset()");
    exit(1);
  }

  //lettura da file
  size_t size = Max_line_len;
  int pindex = 0;

  //produco le righe e le metto nel buff_one
  while(getline(&linea, &size, f) != -1){
    //debug
    printf("[t1]%s", linea);

    xsem_wait(buff_one.sem_free_slots,__LINE__,__FILE__);
    //copio la linea nello slot del buffer
    strcpy((char *)&buff_one.buff[pindex % Buf_size], linea);
    pindex += 1;
    xsem_post(buff_one.sem_data_items,__LINE__,__FILE__);
  }

  //segnalo la terminazione a t2
  strcpy(linea, "-1");
  xsem_wait(buff_one.sem_free_slots,__LINE__,__FILE__);
  strcpy((char *)&buff_one.buff[pindex % Buf_size], linea);
  pindex += 1;
  xsem_post(buff_one.sem_data_items,__LINE__,__FILE__);
  printf("[t1]Segnalo -1\n");

  //join t2 e t3
  xpthread_join(t2, NULL,__LINE__,__FILE__);
  xpthread_join(t3, NULL,__LINE__,__FILE__);

  //distruzione semafori e mutex
  int e = sem_destroy(buff_one.sem_free_slots);
  assert(e==0);
  e = sem_destroy(buff_one.sem_data_items);
  assert(e==0);
  e = sem_destroy(buff_two.sem_free_slots);
  assert(e==0);
  e = sem_destroy(buff_one.sem_data_items);
  assert(e==0);
  xpthread_mutex_destroy(buff_one.mutex, __LINE__, __FILE__);
  xpthread_mutex_destroy(buff_two.mutex, __LINE__, __FILE__);

  //free memoria allocata
  free(buff_one.sem_free_slots);
  free(buff_one.sem_data_items);
  free(buff_one.mutex);
  free(buff_one.buff);
  free(buff_two.sem_free_slots);
  free(buff_two.sem_data_items);
  free(buff_two.mutex);
  free(buff_two.buff);
  free(linea);

  //disallocazione FILE
  e = fclose(f);
  assert(e==0);

  return 0;
}

// funzione eseguita dal thread t2
void *tbody2(void *arg)
{
  threadArg *a = (threadArg *)arg;
  int cindex=0;
  int pindex=0;
  char linea[Max_line_len];
  while(true){
    //consumatore buff_one
      xsem_wait(a->pcbuff1->sem_data_items,__LINE__,__FILE__);
      strcpy(linea, (char *)&a->pcbuff1->buff[cindex % Buf_size]);
      strcpy(linea, (char *)&a->pcbuff1->buff[cindex % Buf_size]);
      cindex += 1;
      xsem_post(a->pcbuff1->sem_free_slots,__LINE__,__FILE__);
      //debug
      printf("[t2]%s", linea);

      //ora divento produttore per il buff_two, devo infilarci la stringa
      //letta da buff_one
      xsem_wait(a->pcbuff2->sem_free_slots,__LINE__,__FILE__);
      strcpy((char *)&a->pcbuff2->buff[pindex % Buf_size], linea);
      pindex += 1;
      xsem_post(a->pcbuff2->sem_data_items,__LINE__,__FILE__);
      if(strcmp(linea, "-1")==0){printf("[t2]break\n");break;}
  }

  pthread_exit(NULL);
}

// funzione eseguita dal thread t3
void *tbody3(void *arg)
{
  threadArg *a = (threadArg *)arg;
  int e;

  //apertura file da scrivere
  //apertura file
  FILE *f = xfopen(a->outputFileName, "w+", __LINE__, __FILE__);
  if(f == NULL){
    perror("[t3]Problema con l'apertura del file");
  }

  //debug
  printf("[t3]Scrittura su %s\n", a->outputFileName);
  //consumatore buff_two
  int cindex=0;
  char linea[Max_line_len];
  while(true){
    xsem_wait(a->pcbuff2->sem_data_items,__LINE__,__FILE__);
    strcpy(linea, (char *)&a->pcbuff2->buff[cindex % Buf_size]);
    cindex += 1;
    xsem_post(a->pcbuff2->sem_free_slots,__LINE__,__FILE__);
    //debug
    printf("[t3]%s", linea);
    if(strcmp(linea, "-1")==0){printf("[t3]break\n");break;}

    //conversione maiuscola minuscola linea
    swapStringCase(linea, strlen(linea));

    //scrittura su file
    fputs(linea, f);
  }
  e = fclose(f);
  assert(e==0);
  pthread_exit(NULL);
}

//sostituisce maiucola minuscola ad una stringa
void swapStringCase(char *s, size_t lenght){
  for(int i=0; i<lenght; i++){
    if(isupper(s[i])){
      s[i] = tolower(s[i]);
    }else{
      s[i] = toupper(s[i]);
    }
  }
}
