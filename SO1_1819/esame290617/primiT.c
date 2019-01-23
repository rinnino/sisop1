#include "xerrors.h"
#include <stdbool.h> // definizione tipo bool

//dimensione buffer pc
#define Buf_size 10

//funzione per test di primalita
bool primo(int n);
//funzione eseguita dai thread consumatori
void *pbody(void *arg);

//dati per i thread consumatori
typedef struct{
  int threadFlag;
  int m; // limite superiore primi da calcolare
  int n; //numero thread produttori
  //buffer pc
  int *buffer;
  int *pindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex_pc;
}datic;

int main(int argc, char const *argv[]) {
  //controllo input
  if(argc!=4) {
    printf("Uso\n\t%s nomefile M N\n", argv[0]);
    exit(1);
  }

  int nThread = atoi(argv[3]);
  int m = atoi(argv[2]);

  //usiamo un bufferPC come schema di comunicazione tra i thread
  datic argT[nThread];
  int bufferPC[Buf_size];
  int pindex=0;
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
    argT[i].threadFlag = i;
    argT[i].m = m;
    argT[i].n = nThread;
    argT[i].buffer = bufferPC;
    argT[i].pindex = &pindex;
    argT[i].sem_free_slots = &sem_free_slots;
    argT[i].sem_data_items = &sem_data_items;
    argT[i].mutex_pc = &mutex_buf;
    xpthread_create(&tp[i], NULL, pbody, (void *) &argT[i], __LINE__, __FILE__);
  }

  //consumatore
  int cindex=0;
  int p; //numero primo consumato dal buffer
  int threadConclusi=0;
  FILE *f = xfopen(argv[1], "w", __LINE__, __FILE__);
  while(threadConclusi<nThread){ //finche ce almeno un thread che produce ...
    xsem_wait(&sem_data_items, __LINE__, __FILE__);
    xpthread_mutex_lock(&mutex_buf, __LINE__, __FILE__);
    p = bufferPC[cindex%Buf_size];
    cindex+=1;
    if(p==-1){
      //un thread produttore è terminato !
      threadConclusi+=1;
    }else{
      //è arrivato un numero primo ! scriviamolo sul file
      fprintf(f, "%d\n", p);
    }
    xpthread_mutex_unlock(&mutex_buf, __LINE__, __FILE__);
    xsem_post(&sem_free_slots, __LINE__, __FILE__);
  }

  //attesa thread
  int figliChiusi=0;
  for(; figliChiusi<nThread; figliChiusi++){
    xpthread_join(tp[figliChiusi], NULL,__LINE__,__FILE__);
  }
  printf("[p]Ho terminato %d thread\n", figliChiusi);

  // fine
  xpthread_mutex_destroy(&mutex_buf,__LINE__,__FILE__);
  fclose(f);
  return 0;
}

void *pbody(void *arg){
  datic *a = (datic *)arg;
  int t; // numero da testare
  t = (2*a->threadFlag + 1)%(2*a->n);
  //produzione
  while(t < a->m){
    printf("[f%d]Testo %d\n", a->threadFlag, t);
    if(primo(t)){
      printf("[f%d]%d e' primo.\n", a->threadFlag, t);
      //produzione
      xsem_wait(a->sem_free_slots,__LINE__,__FILE__);
      xpthread_mutex_lock(a->mutex_pc,__LINE__,__FILE__);
      a->buffer[*(a->pindex) % Buf_size] = t;
      *(a->pindex) += 1;
      xpthread_mutex_unlock(a->mutex_pc,__LINE__,__FILE__);
      xsem_post(a->sem_data_items,__LINE__,__FILE__);
    }
    t+=2*a->n;
  }
  //segnaliamo terminazione produzione
  xsem_wait(a->sem_free_slots,__LINE__,__FILE__);
  xpthread_mutex_lock(a->mutex_pc,__LINE__,__FILE__);
  a->buffer[*(a->pindex) % Buf_size] = -1;
  *(a->pindex) += 1;
  xpthread_mutex_unlock(a->mutex_pc,__LINE__,__FILE__);
  xsem_post(a->sem_data_items,__LINE__,__FILE__);
  pthread_exit(NULL);
}

bool primo(int n)
{
  int i;
  if(n<2) return false;
  if(n%2==0) {
    if(n==2)
      return true;
    else
      return false;
  }
  for (i=3; i*i<=n; i += 2) {
      if(n%i==0) {
          return false;
      }
  }
  return true;
}
