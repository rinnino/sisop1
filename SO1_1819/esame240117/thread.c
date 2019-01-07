/*
* esame del 24/01/17, parte sui thread.
*/
#include "xerrors.h"

#define Buf_size 10

//funzione eseguita dai thread produttori
void *pbody(void *arg);

//dati per i thread produttori
typedef struct{
  FILE *f; //file
  pthread_mutex_t *mutex_file; //mutex accesso file
  //buffer pc
  int *buffer;
  int *pindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex_prod;
}datip;

//main
int main(int argc, char const *argv[]) {
  //controllo input
  if(argc<3) {
    printf("Uso\n\t%s N nomefile\n", argv[0]);
    exit(1);
  }
  int nThread = atoi(argv[1]);
  assert(nThread>0);

  //init buffer pc
  datip argT;
  int bufferPC[Buf_size];
  int pindex=0;
  sem_t sem_free_slots, sem_data_items;
  pthread_mutex_t mutex_buf, mutex_file;

  // inizializzazione semafori
  xsem_init(&sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(&sem_data_items, 0, 0,__LINE__,__FILE__);

  //inizializzazione mutex
  xpthread_mutex_init(&mutex_buf,NULL,__LINE__,__FILE__);
  xpthread_mutex_init(&mutex_file,NULL,__LINE__,__FILE__);

  //init produttori
  pthread_t tp[nThread];
  argT.f = xfopen(argv[2], "r", __LINE__,__FILE__);
  argT.mutex_file = &mutex_file;
  argT.mutex_prod = &mutex_buf;
  argT.buffer = bufferPC;
  argT.pindex = &pindex;
  argT.sem_free_slots = &sem_free_slots;
  argT.sem_data_items = &sem_data_items;
  for(int i=0; i<nThread; i++){
    xpthread_create(&tp[i], NULL, pbody, (void *) &argT, __LINE__, __FILE__);
  }

  //consumatore
  int sum=0; //sommatoria quadrati
  int cindex=0; //indice consumatore
  int prodEnd=0; //produttori che han terminato
  while(prodEnd<nThread){
    xsem_wait(&sem_data_items,__LINE__,__FILE__);
    xpthread_mutex_lock(&mutex_buf,__LINE__,__FILE__);
    if(bufferPC[cindex%Buf_size] ==-1){
        prodEnd+=1;
    }else{
      //printf("consumo %d\n", bufferPC[cindex%Buf_size]);
      sum += bufferPC[cindex%Buf_size];
    }
    cindex += 1;
    xpthread_mutex_unlock(&mutex_buf,__LINE__,__FILE__);
    xsem_post(&sem_free_slots,__LINE__,__FILE__);
  }

  // join dei thread consumatori
  for(int i=0;i<nThread;i++) {
    xpthread_join(tp[i], NULL,__LINE__,__FILE__);
  }

  //chiusura file
  fclose(argT.f);

  printf("Somma: %d\n", sum);

  xpthread_mutex_destroy(&mutex_buf,__LINE__,__FILE__);
  xpthread_mutex_destroy(&mutex_file,__LINE__,__FILE__);

  return 0;
}


//funzione eseguita dai thread produttori
void *pbody(void *arg){
  datip *a = (datip *)arg;
  //apriamo il file
  int e, n;
  do{
    //accesso al file mutualmente esclusivo
    xpthread_mutex_lock(a->mutex_file, __LINE__, __FILE__);
    e = fscanf(a->f, "%d", &n);
    xpthread_mutex_unlock(a->mutex_file, __LINE__, __FILE__);
    if(e!=1){break;} // se il file non ha piu numeri esci
    // produci un unità di lavori nel buffer pc
    xsem_wait(a->sem_free_slots,__LINE__,__FILE__);
    xpthread_mutex_lock(a->mutex_prod,__LINE__,__FILE__);
    a->buffer[*(a->pindex) % Buf_size] = n*n;
    *(a->pindex) += 1;
    xpthread_mutex_unlock(a->mutex_prod,__LINE__,__FILE__);
    xsem_post(a->sem_data_items,__LINE__,__FILE__);
  }while(true);
  //segnalazione fine produzione
  xsem_wait(a->sem_free_slots,__LINE__,__FILE__);
  xpthread_mutex_lock(a->mutex_prod,__LINE__,__FILE__);
  a->buffer[*(a->pindex) % Buf_size] = -1;
  xpthread_mutex_unlock(a->mutex_prod,__LINE__,__FILE__);
  *(a->pindex) += 1;
  xsem_post(a->sem_data_items,__LINE__,__FILE__);
  pthread_exit(NULL);
}
