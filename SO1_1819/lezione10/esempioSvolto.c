/*
 * Esempio compless paradigma produttore consumatori
 *
 * Fare riferimento alla lezione 10 su DIR per verdere il testo dell'esercizio
 * */
#include "xerrors.h"

#define Buf_size 10


// funzione per stabilire se n e' primo
bool primo(int n)
{
  if(n<2) return false;
  if(n%2==0) return (n==2);
  for (int i=3; i*i<=n; i += 2)
      if(n%i==0) return false;
  return true;
}

// struct contenente i parametri di input e output dei thread consiumatori
typedef struct {
  FILE *file;   // output
  int *buffer;
  int *pcindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex_buf;
  pthread_mutex_t *mutex_file;
} dati;

// struct contenente i parametri per i thread produttori
typedef struct {
  char *nomefile;
  int *buffer;
  int *ppindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
  pthread_mutex_t *mutex_prod;
} datip;

// funzione eseguita dai thread producer
void *pbody(void *arg)
{
  datip *a = (datip *)arg;
  int n;
  FILE *f = xfopen(a->nomefile,"rt",__LINE__,__FILE__);
  do {
    int e  = fscanf(f,"%d",&n);
    if(e!=1) break;
    xsem_wait(a->sem_free_slots,__LINE__,__FILE__);
    xpthread_mutex_lock(a->mutex_prod,__LINE__,__FILE__);
    a->buffer[*(a->ppindex) % Buf_size] = n;
    *(a->ppindex) += 1;
    xpthread_mutex_unlock(a->mutex_prod,__LINE__,__FILE__);
    xsem_post(a->sem_data_items,__LINE__,__FILE__);
  } while(true);
  fclose(f);
  pthread_exit(NULL);
}

int divisori(int n) {
  int div = 2;
  for(int i=2;i<n;i++)
    if(n%i==0) div++;
  return div;
}


// funzione eseguita dai thread consumer
void *tbody(void *arg)
{
  dati *a = (dati *)arg;
  int n;
  do {
    xsem_wait(a->sem_data_items,__LINE__,__FILE__);
    xpthread_mutex_lock(a->mutex_buf,__LINE__,__FILE__);
    n = a->buffer[*(a->pcindex) % Buf_size];
    *(a->pcindex) += 1;
    xpthread_mutex_unlock(a->mutex_buf,__LINE__,__FILE__);
    xsem_post(a->sem_free_slots,__LINE__,__FILE__);
    if(n!=-1) {
      int d = divisori(n);
      xpthread_mutex_lock(a->mutex_file,__LINE__,__FILE__);
      fprintf(a->file,"%d %d\n",n,d);
      xpthread_mutex_unlock(a->mutex_file,__LINE__,__FILE__);
    }
  } while(n!= -1);
  pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
  // leggi input
  if(argc<4) {
    printf("Uso\n\t%s file1 file2 ... outfile numthread\n", argv[0]);
    exit(1);
  }
  int p = atoi(argv[argc-1]);
  assert(p>=0);
  // threads related
  int buffer[Buf_size];
  int cindex=0, pindex=0;
  pthread_t t[p];
  dati a[p];
  sem_t sem_free_slots, sem_data_items;
  pthread_mutex_t mutex_buf, mutex_file, mutex_prod;
  // inizializzazione semafori
  xsem_init(&sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(&sem_data_items, 0, 0,__LINE__,__FILE__);
  //inizializzazione mutex
  xpthread_mutex_init(&mutex_buf,NULL,__LINE__,__FILE__);
  xpthread_mutex_init(&mutex_file,NULL,__LINE__,__FILE__);
  xpthread_mutex_init(&mutex_prod,NULL,__LINE__,__FILE__);

  // inizializzazione  produttori
  int nump = argc -3;
  pthread_t tp[nump];
  datip ap[nump];
  // creazione thread
  for(int i=0;i<nump;i++) {
    ap[i].buffer = buffer;
    ap[i].ppindex = &pindex;
    ap[i].nomefile = argv[i+1];
    ap[i].mutex_prod = &mutex_prod;
    ap[i].sem_data_items  = &sem_data_items;
    ap[i].sem_free_slots  = &sem_free_slots;
    xpthread_create(&tp[i], NULL, pbody, (void *) &ap[i],__LINE__,__FILE__);
  }

  // inzializzazione consumatori
  // apro il file di output
  FILE *f = xfopen(argv[argc-2],"wt",__LINE__,__FILE__);
  //  creo i thread consumatori
  for(int i=0;i<p;i++) {
    a[i].buffer = buffer;
    a[i].pcindex = &cindex;
    a[i].file = f;
    a[i].mutex_buf = &mutex_buf;
    a[i].mutex_file = &mutex_file;
    a[i].sem_data_items  = &sem_data_items;
    a[i].sem_free_slots  = &sem_free_slots;
    xpthread_create(&t[i], NULL, tbody, (void *) &a[i],__LINE__,__FILE__);
  }

  // join dei thread produttori
  for(int i=0;i<nump;i++) {
    xpthread_join(tp[i], NULL,__LINE__,__FILE__);
  }
  // segnalo terminazione ai consumatori
  for(int i=0;i<p;i++) {
    xsem_wait(&sem_free_slots,__LINE__,__FILE__);
    buffer[pindex % Buf_size] = -1;
    pindex += 1;
    xsem_post(&sem_data_items,__LINE__,__FILE__);
  }
  // join dei thread consumatori
  for(int i=0;i<p;i++) {
    xpthread_join(t[i], NULL,__LINE__,__FILE__);
  }
  fclose(f);
  xpthread_mutex_destroy(&mutex_buf,__LINE__,__FILE__);
  xpthread_mutex_destroy(&mutex_file,__LINE__,__FILE__);
  xpthread_mutex_destroy(&mutex_prod,__LINE__,__FILE__);
  return 0;
}
