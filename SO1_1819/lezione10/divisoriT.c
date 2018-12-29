#include "xerrors.h"
#define Buf_size 10
#define Max_line_len 80

//uso : divisoriT infile1 infile2 infile3 ... infileN outfile numt

//crea argc-1 produttori che leggono i rispettivi file infileX e mettono il
//numero nel bufferPC.
//poi il thread principale creera numT thread consumatori che scriveranno su
//outfile il numero ed accanto il numero di divisori di quel numero.

//struttura dati per buffer produttore consumatore

// funzione eseguita dal thread produttore
void *tbodyp(void *arg);
// funzione eseguita dal thread consumatore
void *tbodyc(void *arg);
//restituisce il numero di divisori di n
int divisori(int n);

typedef struct {
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex;
  int *buff;
  int buffSize;
  int pindex;
  int cindex;
} pcBuffer;

//struttura dati per thread produttore
typedef struct{
  const char *inFile;
  pcBuffer *pcbuff;
  int threadFlag;
}threadArgp;

//struttura dati per thread consumatore
typedef struct{
  FILE *outFile;
  pcBuffer *pcbuff;
  int threadFlag;
  pthread_mutex_t *mutexFile;
}threadArgc;

int main(int argc, char const *argv[]) {
  if(argc<4) {
    printf("Uso\n\t%s infile1 infile2 infile3 ... infileN outfile numt\n", argv[0]);
    exit(1);
  }

  //init buffer produttore consumatore
  pcBuffer buff_pc;
  buff_pc.sem_free_slots = malloc(sizeof(sem_t));
  buff_pc.sem_data_items = malloc(sizeof(sem_t));
  buff_pc.mutex = malloc(sizeof(pthread_mutex_t));
  buff_pc.buff = malloc(sizeof(int)*Buf_size);
  buff_pc.buffSize = Buf_size;
  buff_pc.cindex = 0;
  buff_pc.pindex = 0;

  //init semafori del buffer
  xsem_init(buff_pc.sem_free_slots, 0, buff_pc.buffSize, __LINE__, __FILE__);
  xsem_init(buff_pc.sem_data_items, 0, 0, __LINE__, __FILE__);

  //init mutex del buffer
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
  xpthread_mutex_init(buff_pc.mutex, &attr, __LINE__, __FILE__);

  //init mutex del file di output
  pthread_mutex_t *mutexFile = malloc(sizeof(pthread_mutex_t));
  xpthread_mutex_init(mutexFile, &attr, __LINE__, __FILE__);

  //thread produttori
  pthread_t produttori[argc-3];
  //debug
  printf("[t1]Faccio partire %d produttori\n", argc-3);

  //prepariamo gli argomenti da passare ai thread produttori
  threadArgp arg[argc-3];
  //debug
  printf("[t1]Il file di output sarà %s\n", argv[argc-2]);
  //lanciamo i thread produttori
  for(int i=0; i<argc-3; i++){
    //debug
    printf("[t1]Invoco il thread tp%d che lavorera sul file %s\n", i+1, argv[i+1]);
    arg[i].pcbuff = &buff_pc;
    arg[i].inFile = argv[i+1];
    arg[i].threadFlag = i+1;
    xpthread_create(&produttori[i], NULL, tbodyp, (void *) &arg[i], __LINE__, __FILE__);
  }

  int nConsumatori = atoi(argv[argc-1]);

  //thread consumatori
  pthread_t consumatori[nConsumatori]; //numt thread consumatori

  //prepariamo gli argomenti da passare ai thread
  threadArgc argcons[nConsumatori];

  //apriamo il file di output
  FILE *f = xfopen(argv[argc-2], "wt", __LINE__, __FILE__);
  if(f == NULL){
    perror("[t1]Problema con l'apertura del file di output");
  }


  //lanciamo i thread consumatori
  for(int i=0; i<nConsumatori; i++){
    argcons[i].outFile = f;
    argcons[i].pcbuff = &buff_pc;
    argcons[i].threadFlag = i+1;
    argcons[i].outFile = f;
    argcons[i].mutexFile = mutexFile;
    xpthread_create(&consumatori[i], NULL, tbodyc, (void *) &argcons[i], __LINE__, __FILE__);
  }

  //join dei thread produttori
  for(int i=0; i<argc-3; i++){
    xpthread_join(produttori[i], NULL,__LINE__,__FILE__);
  }

  //segnalazione terminazione ai thread consumatori
  for(int i=0; i<nConsumatori; i++){
    xsem_wait(buff_pc.sem_free_slots, __LINE__, __FILE__);
    xpthread_mutex_lock(buff_pc.mutex, __LINE__, __FILE__);
    buff_pc.buff[buff_pc.pindex%buff_pc.buffSize]=-1;
    buff_pc.pindex+=1;
    xpthread_mutex_unlock(buff_pc.mutex, __LINE__, __FILE__);
    xsem_post(buff_pc.sem_data_items, __LINE__, __FILE__);
  }

  //join threads consumatori
  for(int i=0; i<nConsumatori; i++){
    xpthread_join(consumatori[i], NULL,__LINE__,__FILE__);
  }

  //distruziome semafori e mutex del buffer
  int e = sem_destroy(buff_pc.sem_free_slots);
  assert(e==0);
  e = sem_destroy(buff_pc.sem_data_items);
  assert(e==0);
  xpthread_mutex_destroy(buff_pc.mutex, __LINE__, __FILE__);
  xpthread_mutex_destroy(mutexFile, __LINE__, __FILE__);

  //free memoria allocata
  free(buff_pc.sem_free_slots);
  free(buff_pc.sem_data_items);
  free(buff_pc.mutex);
  free(buff_pc.buff);
  free(mutexFile);

  //chiusura file output
  e = fclose(f);
  assert(e==0);

  return 0;
}

// funzione eseguita dal thread produttore
void *tbodyp(void *arg){
  threadArgp *a = (threadArgp *)arg;
  //debug
  printf("[tp%d]Ciao, io sono il produttore del file %s\n", a->threadFlag,
                                                            a->inFile);
  //apriamo il FILE
  FILE *f = xfopen(a->inFile, "r", __LINE__, __FILE__);
  if(f == NULL){
    perror("[tpx]Problema con l'apertura del file ");
  }
  //predisposizione buffer di lettura
  char *linea = malloc(Max_line_len);
  if(linea == NULL){
    perror("Problema malloc\n");
    exit(1);
  }
  if(memset(linea, 0, Max_line_len) == NULL){
    perror("[tpx]Problema con memset()");
    exit(1);
  }
  //lettura da file
  size_t size = Max_line_len;
  int n;
  while(getline(&linea, &size, f)!=-1){
    //debug
    //printf("[tp%d]%s", a->threadFlag, linea);

    //produzione: inserisco il numero appena letto nel buffer condiviso
    n = atoi(linea);
    xsem_wait(a->pcbuff->sem_free_slots, __LINE__, __FILE__);
    xpthread_mutex_lock(a->pcbuff->mutex, __LINE__, __FILE__);
    a->pcbuff->buff[(a->pcbuff->pindex)%(a->pcbuff->buffSize)] = n;
    //debug
    printf("[tp%d]Inserisco %d nella posizione %d\n", a->threadFlag,
                a->pcbuff->buff[(a->pcbuff->pindex)%(a->pcbuff->buffSize)],
                (a->pcbuff->pindex)%(a->pcbuff->buffSize));
    a->pcbuff->pindex+=1;
    xpthread_mutex_unlock(a->pcbuff->mutex, __LINE__, __FILE__);
    xsem_post(a->pcbuff->sem_data_items, __LINE__, __FILE__);
  }
  free(linea);
  int e = fclose(f);
  assert(e==0);
  pthread_exit(NULL);
}

// funzione eseguita dal thread consumatore
void *tbodyc(void *arg){
  threadArgc *a = (threadArgc *)arg;
  int n;
  //consumatore: prelevo un'intero dal buffer e lo scrivo su file affianco
  // al suo numero di divisori
  while(true){
    xsem_wait(a->pcbuff->sem_data_items, __LINE__, __FILE__);
    xpthread_mutex_lock(a->pcbuff->mutex, __LINE__, __FILE__);
    n = a->pcbuff->buff[(a->pcbuff->cindex)%(a->pcbuff->buffSize)];
    a->pcbuff->cindex+=1;
    xpthread_mutex_unlock(a->pcbuff->mutex, __LINE__, __FILE__);
    xsem_post(a->pcbuff->sem_free_slots, __LINE__, __FILE__);
    //debug
    printf("[tc%d]Consumo %d\n", a->threadFlag, n );
    if(n!=-1){
      xpthread_mutex_lock(a->mutexFile, __LINE__, __FILE__);
      fprintf(a->outFile, "%d %d\n", n, divisori(n));
      xpthread_mutex_unlock(a->mutexFile, __LINE__,__FILE__);
    }else{
      break;
    }
  }

  pthread_exit(NULL);
}

//restituisce il numero di divisori di n
int divisori(int n){
  int div = 2;
  for(int i=2;i<n;i++)
    if(n%i==0) div++;
  return div;
}
