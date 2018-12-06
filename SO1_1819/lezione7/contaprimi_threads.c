#include "xerrors.h"

// programma per il conteggio di numero dei primi in un
// intervallo usando piu' thread

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


int cerca(int a, int b)
{
  int tot = 0;
  for(int i=a;i<b;i++)
    if(primo(i)) tot++;
  return tot;  
}

// struttura usata per comunicare con i thread
typedef struct {
  int start;   
  int end;   
  int tot;  
} dati; 



void *tbody(void *arg)
{
  dati *d = (dati *) arg;   
  d->tot = cerca(d->start,d->end);
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

  pthread_t t[p];    // usata internamente per gestire i thread
  dati d[p];         // usato per passare input e output  
  for(int i=0;i<p;i++) {
     int n = (n1-n0)/p;  // quanti numeri verifica ogni figlio + o - 
     int start = n0 + n*i;
     int end = (i==p-1) ? n1 : n0 + n*(i+1); 
	 d[i].start = start;	
	 d[i].end = end;	
     xpthread_create(&t[i],NULL,tbody,&d[i],__LINE__,__FILE__);  
  }
  // attende terminazione dei thread 
  int tot=0;
  for(int i=0;i<p;i++) {
    int e = pthread_join(t[i], NULL);  //\\ il valore passato dalla exit viene ignorato
    if(e!=0) {
      fprintf(stderr,"Errore pthread_join (%d)\n",e); 
      exit(1);
    }
    printf("Thread %d ha restituito %d\n",i,d[i].tot);
    tot += d[i].tot;
  }
  printf("Numero primi p tali che  %d <= p < %d è: %d\n",n0,n1,tot);
  return 0;
}
