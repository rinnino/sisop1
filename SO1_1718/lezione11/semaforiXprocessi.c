/*
 * Esempio semplice paradigma produttore consumatori realizzato con i processi
 * Deve essere linkato con xerrors.o e l'opzion -pthread
 * 
 * Usare numeri.py per generare lunghi elenchi di interi positivi su cui testare il programma
 * 
 * Programma di esempio del paradigma 1 producer N consumer
 * i dati letti dal file vengono messi su un buffer in cui il producer scrive 
 * e i consumer leggono. In principio il buffer va bene di qualsiasi dimensione: 
 * piu' e' grande maggiore e' il lavoro pronto da svolgere nel caso
 * il produttore rimanga bloccato (ad esempio a leggere dal disco)
 * 
 * */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>      
#include <sys/types.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "xerrors.h"
#include <sys/ipc.h>
#include <sys/shm.h>


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


int main(int argc, char *argv[])
{
  // leggi input
  if(argc!=3) {
    printf("Uso\n\t%s file numproc\n", argv[0]);
    exit(1);
  }
  int p = atoi(argv[2]);
  assert(p>=0);
  int tot_primi = 0;
  int tot_somma = 0;
  int i,e,n;    
  // alloca i semafori nella shared memory 
  int shmid = xshmget(IPC_PRIVATE,3*sizeof(sem_t),0600,__LINE__,__FILE__);
  sem_t *a = (sem_t *) xshmat(shmid,NULL,0,__LINE__,__FILE__);
  sem_t *free_slots = &a[0]; 
  sem_t *data_items = &a[1]; 
  sem_t *mutex_consumers = &a[2]; 
  // inizializza i semafori, il secondo argomento !=0 permette di condividerli tra processi
  xsem_init(free_slots,1,Buf_size,__LINE__,__FILE__);
  xsem_init(data_items,1,0,__LINE__,__FILE__);
  xsem_init(mutex_consumers,1,1,__LINE__,__FILE__);  
  // inizializza shared memory per contenere buffer, somma, quanti e cindex
  int shmid2 = xshmget(IPC_PRIVATE,(2*p+Buf_size+1)*sizeof(int),0600,__LINE__,__FILE__);
  int *buffer = (int *) xshmat(shmid2,NULL,0,__LINE__,__FILE__);  
  int *somma = buffer + Buf_size;
  int *quanti = somma + p;
  int *cindex = quanti + p;
  *cindex=0; 
  int pindex=0;

  //  creo i processi consumatori
  for(i=0;i<p;i++) {
    if(xfork(__LINE__,__FILE__)==0) {
      while(true) { // codice processi consumatori
        xsem_wait(data_items,__LINE__,__FILE__);
        xsem_wait(mutex_consumers,__LINE__,__FILE__);
        int n = buffer[*cindex];
        *cindex = (*cindex+1)%Buf_size;
        xsem_post(mutex_consumers,__LINE__,__FILE__);
        xsem_post(free_slots,__LINE__,__FILE__);        
        if(n<0) break;
        if(primo(n)) {
          quanti[i] +=1;
          somma[i] += n;
        }
      }
      // detach memoria condivisa
      xshmdt(a,__LINE__,__FILE__);
      xshmdt(buffer,__LINE__,__FILE__);
      exit(0);
    }
  }
  // codice processo produttore
  FILE *f = fopen(argv[1],"r");
  if(f==NULL) {perror("Errore apertura file"); return 1;}
  while(true) {
    e = fscanf(f,"%d", &n);
    if(e!=1) break; // se il valore e' letto correttamente e==1
    assert(n>0);    // i valori del file devono essere positivi
    // metto n nel buffer
    xsem_wait(free_slots,__LINE__,__FILE__);
    buffer[pindex] = n;
    pindex = (pindex+1)%Buf_size;
    xsem_post(data_items,__LINE__,__FILE__);
  }
  // segnalo terminazione ai consumatori
  for(int i=0;i<p;i++) {
    xsem_wait(free_slots,__LINE__,__FILE__);
    buffer[pindex] = -1;
    pindex = (pindex+1)%Buf_size;
    xsem_post(data_items,__LINE__,__FILE__);
  }
  // aspetta terminazione figli
  for(int i=0;i<p;i++) { 
    xwait(NULL,__LINE__,__FILE__);
  }
  // calcolo risultato finale
  tot_somma = tot_primi = 0;
  for(int i=0;i<p;i++) { 
    tot_somma += somma[i];
    tot_primi += quanti[i];
  }
  
  fprintf(stderr,"Trovati %d primi con somma %d\n",tot_primi,tot_somma);
  // detach shared memory 
  xshmdt(a,__LINE__,__FILE__);
  xshmdt(buffer,__LINE__,__FILE__);
  // rimozione shared memory
  xshmctl(shmid, IPC_RMID,  NULL,__LINE__,__FILE__);
  xshmctl(shmid2, IPC_RMID,  NULL,__LINE__,__FILE__);
  return 0;
}
