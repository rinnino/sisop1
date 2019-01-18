#include "xerrors.h"

//dimensione buffer pc
#define Buf_size 10
#define MAX_FILE_NAME 100

//elevazione a potenza x^y, valida solo per y>=0
long pot(int x, int y);

//funzione eseguita dai thread consumatori
void *cbody(void *arg);

//dati per i thread consumatori
typedef struct{
  char const *file; //file
  int threadFlag;
  //buffer pc
  int *buffer;
  int *cindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex_pc;
}datic;

int main(int argc, char const *argv[]) {
  //controllo input
  if(argc<3) {
    printf("Uso\n\t%s N nomefile\n", argv[0]);
    exit(1);
  }
  int nThread = atoi(argv[3]);
  assert(nThread>0);
  int maxExp = atoi(argv[2]);

  datic argT[nThread];
  int bufferPC[Buf_size];
  int cindex=0;
  sem_t sem_free_slots, sem_data_items;
  pthread_mutex_t mutex_buf;

  // inizializzazione semafori
  xsem_init(&sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(&sem_data_items, 0, 0,__LINE__,__FILE__);

  //inizializzazione mutex
  xpthread_mutex_init(&mutex_buf,NULL,__LINE__,__FILE__);

  //init thread consumatori
  pthread_t tp[nThread];
  for(int i=0; i<nThread; i++){
    argT[i].file = argv[1];
    argT[i].threadFlag = i;
    argT[i].buffer = bufferPC;
    argT[i].cindex = &cindex;
    argT[i].sem_free_slots = &sem_free_slots;
    argT[i].sem_data_items = &sem_data_items;
    argT[i].mutex_pc = &mutex_buf;
    xpthread_create(&tp[i], NULL, cbody, (void *) &argT[i], __LINE__, __FILE__);
  }

  //produttore esponenti
  int pindex=0;
  for(int i=1; i<=maxExp; i++){
    xsem_wait(&sem_free_slots, __LINE__, __FILE__);
    xpthread_mutex_lock(&mutex_buf, __LINE__, __FILE__);
    bufferPC[pindex%Buf_size] = i;
    pindex+=1;
    xpthread_mutex_unlock(&mutex_buf, __LINE__, __FILE__);
    xsem_post(&sem_data_items, __LINE__, __FILE__);
    printf("[p]Ho prodotto l'esponente %d\n", i);
  }
  //segnala terminazione
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


  xpthread_mutex_destroy(&mutex_buf,__LINE__,__FILE__);

  return 0;
}

long pot(int x, int y){
  if(y==0) return 1;
  if(y==1) return x;
  long ris = x;
  for(int i=1; i<y; i++){ris=ris*x;}
  return ris;
}

void *cbody(void *arg){
  datic *a = (datic *)arg;
  int exp;
  char nomeFileOut[MAX_FILE_NAME];
  int e, n;
  //consumatore esponenti
  FILE *f_in = xfopen(a->file, "r", __LINE__, __FILE__); //file input
  FILE *f_out; //file output
  while(true){
    xsem_wait(a->sem_data_items,__LINE__,__FILE__);
    xpthread_mutex_lock(a->mutex_pc,__LINE__,__FILE__);
    exp = a->buffer[*(a->cindex) % Buf_size];
    *(a->cindex) += 1;
    xpthread_mutex_unlock(a->mutex_pc,__LINE__,__FILE__);
    xsem_post(a->sem_free_slots,__LINE__,__FILE__);
    if(exp==-1){break;}//fine consumatore
    sprintf(nomeFileOut, "%s.%d", a->file, exp);
    f_out = xfopen(nomeFileOut, "w", __LINE__, __FILE__); //apro nomefile.exp
    //scrittura file
    while(true){
      e = fscanf(f_in,"%d",&n); //leggo dal file input
      if(e!=1) break; //fine file
      fprintf(f_out, "%ld\n", pot(n, exp)); //scrivo il numero elevato a potenza
    }
    //chiudo il file output
    fclose(f_out);
    //rewind file input
    rewind(f_in);
  }
  //chiusura file input
  fclose(f_in);
  pthread_exit(NULL);
}
