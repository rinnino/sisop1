#include "xerrors.h"

// programma per la creazione della tabella dei primi in un
// intervallo usando piu' thread

// !!! da completare aggiungendo un semaforo per regolare
// !!! l'accesso alle variabili condivisa tabella e tab_messi

#define MAX_TABELLA 10000

// restituisce true/false a seconda che n sia primo o composto
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


// struttura usata per comunicare con i thread
typedef struct {
  int start;   
  int end;
  int *tabella;   
  int *tab_messi;   
  int tot;
} dati; 



void *tbody(void *arg)
{
  dati *d = (dati *) arg;   
  d->tot = 0;
  for(int i=d->start;i<d->end;i++)
    if(primo(i)) {
      d->tot++;
      // necessario mettere sem_wait
      if(*(d->tab_messi)>= MAX_TABELLA) die("Tabella piena"); 
      d->tabella[*(d->tab_messi)] = i;
      *(d->tab_messi) += 1;
      // necessario mettere sem_post
    }  
  pthread_exit(NULL);    // non restituisce nulla attraverso la exit
}


// conta quanti sono i primi tra argv[1] (compreso) e argv[2] (escluso)
int main(int argc, char *argv[]) {
  if(argc!=4) {
    printf("Uso:\n\t%s n1 n2 p\n",argv[0]);
    exit(1);
  }
  int n0 = atoi(argv[1]);
  int n1 = atoi(argv[2]);
  int p = atoi(argv[3]);
  assert(n0>0 && n1>=n0 && p>0);

  int tabella[MAX_TABELLA]; // brutto ma veloce
  int tab_messi = 0;   
  pthread_t t[p];    // usato internamente per gestire i thread
  dati d[p];         // usato per passare input e output ai thread
  for(int i=0;i<p;i++) {
    int n = (n1-n0)/p;  // quanti numeri verifica ogni figlio + o - 
    int start = n0 + n*i;
    int end = (i==p-1) ? n1 : n0 + n*(i+1); 
    d[i].start = start; 
    d[i].end = end;
    d[i].tabella = tabella;
    d[i].tab_messi = &tab_messi;  
    xpthread_create(&t[i],NULL,tbody,&d[i],__LINE__,__FILE__);  
  }
  // attende terminazione dei thread 
  int tot=0;
  for(int i=0;i<p;i++) {
    xpthread_join(t[i], NULL,__LINE__,__FILE__);
    printf("Thread %d ha restituito %d\n",i,d[i].tot);
    tot += d[i].tot;
  }
  printf("Numero primi p tali che  %d <= p < %d è: %d\n",n0,n1,tot);
  printf("Numero primi in tabella: %d\n",tab_messi);
  return 0;
}
