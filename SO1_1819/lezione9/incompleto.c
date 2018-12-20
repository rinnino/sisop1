/*
 * Esempio semplice paradigma produttore consumatori
 * 
 * Usare il numeri.py per generare lunghi file con interi positivi su cui testare il programma
 * 
 * 
 * Il programma e' incompleto in quanto manca un meccanismo che regoli l'accesso
 * contemporaneo alle variabili pcindex e buffer da parte dei cosumatori
 * (vedere nota nel codice)
 * 
 * Per esercizio risolvere questo problema usando un semaforo deicato alla scopo
 * 
 * Programma di esempio del paradigma 1 producer N consumer
 * i dati letti dal file vengono messi su un buffer in cui il producer scrive 
 * e i consumer leggono. In principio il buffer va bene di qualsiasi dimensione: 
 * piu' e' grande maggiore e' il lavoro pronto da svolgere nel caso
 * il produttore rimanga bloccato (ad esempio a leggere dal disco)
 * 
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

// struct contenente i parametri di input e output di ogni thread 
typedef struct {
  int quanti;   // output
  long somma;   // output
  int *buffer; 
  int *pcindex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;  
} dati;



// funzione eseguita dai thread consumer
void *tbody(void *arg)
{  
  dati *a = (dati *)arg;
  int n;
  a->quanti = a->somma = 0;
  do {
    xsem_wait(a->sem_data_items,__LINE__,__FILE__);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // zona di codice critico in cui piu' consumatori possono accedere
    // in lettura e scrittura alla stessa variabile a->pcindex
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    n = a->buffer[*(a->pcindex) % Buf_size];
    *(a->pcindex) += 1;
    xsem_post(a->sem_free_slots,__LINE__,__FILE__);
    if(n>=0 && primo(n)) {
      a->quanti += 1;
      a->somma += n;
    }
  } while(n!= -1);
  pthread_exit(NULL); 
}     


int main(int argc, char *argv[])
{
  // leggi input
  if(argc!=3) {
    printf("Uso\n\t%s file numthread\n", argv[0]);
    exit(1);
  }
  int p = atoi(argv[2]);
  assert(p>=0);
  int tot_primi = 0;
  long tot_somma = 0;
  int i,e,n;    
  // threads related
  int buffer[Buf_size];
  int cindex=0, pindex=0;
  pthread_t t[p];
  dati a[p];
  sem_t sem_free_slots, sem_data_items;
  
  // inizializzazione semafori
  xsem_init(&sem_free_slots, 0, Buf_size,__LINE__,__FILE__);
  xsem_init(&sem_data_items, 0, 0,__LINE__,__FILE__);
  //  creo i thread consumatori
  for(i=0;i<p;i++) {
    a[i].buffer = buffer;
    a[i].pcindex = &cindex;
    a[i].sem_data_items  = &sem_data_items;
    a[i].sem_free_slots  = &sem_free_slots;
    xpthread_create(&t[i], NULL, tbody, (void *) &a[i],__LINE__,__FILE__);
  }

  // read file and count primes 
  FILE *f = fopen(argv[1],"r");
  if(f==NULL) {perror("Errore apertura file"); return 1;}
  while(true) {
    e = fscanf(f,"%d", &n);
    if(e!=1) break; // se il valore e' letto correttamente e==1
    assert(n>0);    // i valori del file devono essere positivi
    // se p==0 non usa i thread consumatori
    if(p==0) {
      if(primo(n)) {
        tot_primi += 1;
        tot_somma += n;
      }
    }
    else {
      xsem_wait(&sem_free_slots,__LINE__,__FILE__);
      buffer[pindex % Buf_size] = n;
      pindex += 1;
      xsem_post(&sem_data_items,__LINE__,__FILE__);
    }
  }
  // segnalo terminazione
  for(int i=0;i<p;i++) {
    xsem_wait(&sem_free_slots,__LINE__,__FILE__);
    buffer[pindex % Buf_size] = -1;
    pindex += 1;
    xsem_post(&sem_data_items,__LINE__,__FILE__);
  }

  // join dei thread e calcolo risultato
  for(i=0;i<p;i++) {
    xpthread_join(t[i], NULL,__LINE__,__FILE__);
    tot_primi += a[i].quanti;
    tot_somma += a[i].somma;
  }

  fprintf(stderr,"Trovati %d primi con somma %ld\n",tot_primi,tot_somma);
  return 0;
}

