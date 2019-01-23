#include "xerrors.h"
#define Buf_size 10
#define Max_line_len 80

//restituisce il numero di divisori di n
int divisori(int n);

//funzione eseguita dai thread consumatori
void *cbody(void *arg);

//dati per i thread consumatori
typedef struct{
  int threadFlag;
  //buffer pc
  int *buffer;
  int *cindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex_pc;
  //file output
  FILE *outfile;
  pthread_mutex_t *mutex_file;
}datic;

int main(int argc, char const *argv[]) {
  //controllo input
  if(argc!=4) {
    printf("Uso\n\t%s infile outfile numt\n", argv[0]);
    exit(1);
  }

  int nThread = atoi(argv[3]);

  //usiamo un bufferPC come schema di comunicazione tra i thread
  datic argT[nThread];
  int bufferPC[Buf_size];
  int cindex=0;
  sem_t sem_free_slots, sem_data_items;
  pthread_mutex_t mutex_buf;

  //necessitiamo di un mutex condiviso fra i thread consumatori per il file
  pthread_mutex_t mutex_file;

  // inizializzazione semafori
  xsem_init(&sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(&sem_data_items, 0, 0,__LINE__,__FILE__);

  //inizializzazione mutex
  xpthread_mutex_init(&mutex_buf,NULL,__LINE__,__FILE__);
  xpthread_mutex_init(&mutex_file,NULL,__LINE__,__FILE__);

  //init thread consumatori
  pthread_t tp[nThread];
  FILE *f_out = xfopen(argv[2], "w", __LINE__, __FILE__);
  for(int i=0; i<nThread; i++){
    argT[i].threadFlag = i;
    argT[i].buffer = bufferPC;
    argT[i].cindex = &cindex;
    argT[i].sem_free_slots = &sem_free_slots;
    argT[i].sem_data_items = &sem_data_items;
    argT[i].mutex_pc = &mutex_buf;
    argT[i].mutex_file = &mutex_file;
    argT[i].outfile = f_out;
    xpthread_create(&tp[i], NULL, cbody, (void *) &argT[i], __LINE__, __FILE__);
  }

  //produttore
  int e, n;
  int pindex=0;
  FILE *f_in = xfopen(argv[1], "r", __LINE__, __FILE__);
  while(true){
    e = fscanf(f_in, "%d", &n);
    if(e!=1){break;} // se il file non ha piu numeri esci
    //printf("[p]Leggo %d da %s\n", n, argv[1]);
    //produco
    xsem_wait(&sem_free_slots, __LINE__, __FILE__);
    xpthread_mutex_lock(&mutex_buf, __LINE__, __FILE__);
    bufferPC[pindex%Buf_size] = n;
    pindex+=1;
    xpthread_mutex_unlock(&mutex_buf, __LINE__, __FILE__);
    xsem_post(&sem_data_items, __LINE__, __FILE__);
  }
  fclose(f_in);

  //segnalo terminazione ai thread
  for(int i=0; i<nThread; i++){
    xsem_wait(&sem_free_slots, __LINE__, __FILE__);
    xpthread_mutex_lock(&mutex_buf, __LINE__, __FILE__);
    bufferPC[pindex%Buf_size] = -1;
    pindex+=1;
    xpthread_mutex_unlock(&mutex_buf, __LINE__, __FILE__);
    xsem_post(&sem_data_items, __LINE__, __FILE__);
  }

  //attesa thread
  int figliChiusi=0;
  for(; figliChiusi<nThread; figliChiusi++){
    xpthread_join(tp[figliChiusi], NULL,__LINE__,__FILE__);
  }
  printf("[p]Ho terminato %d thread\n", figliChiusi);
  fclose(f_out);

  xpthread_mutex_destroy(&mutex_buf,__LINE__,__FILE__);
  xpthread_mutex_destroy(&mutex_file,__LINE__,__FILE__);

  return 0;
}

int divisori(int n){
  int div = 2;
  for(int i=2;i<n;i++)
    if(n%i==0) div++;
  return div;
}


void *cbody(void *arg){
  datic *a = (datic *)arg;
  int n;
  while(true){
    //consumatore
    xsem_wait(a->sem_data_items,__LINE__,__FILE__);
    xpthread_mutex_lock(a->mutex_pc,__LINE__,__FILE__);
    n = a->buffer[*(a->cindex) % Buf_size];
    *(a->cindex) += 1;
    xpthread_mutex_unlock(a->mutex_pc,__LINE__,__FILE__);
    xsem_post(a->sem_free_slots,__LINE__,__FILE__);
    if(n==-1){break;}//fine consumatore
    //scrivo su file
    xpthread_mutex_lock(a->mutex_file,__LINE__,__FILE__);
    fprintf(a->outfile, "%d %d\n", n, divisori(n));
    xpthread_mutex_unlock(a->mutex_file,__LINE__,__FILE__);
  }
  pthread_exit(NULL);
}
