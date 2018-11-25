#include "xerrori.h"

// programma per il conteggio di numero dei primi in un
// intervallo usando un unico processo e le tecniche di Prog 1

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
  // creo una pipe di comunicazione tra padre e figli
  int up[2]; // la chiamo up perche' la uso da figli a padre
  int e = pipe(up);
  if(e<0) die("Pipe fallita");
  for(int i=0;i<p;i++) {
    pid_t pid = xfork(__FILE__,__LINE__);
    if(pid==0) {// figlio
      close(up[0]);
      int n = (n1-n0)/p;  // quanti numeri verifica ogni figlio + o - 
      int start = n0 + n*i;
      int end = (i==p-1) ? n1 : n0 + n*(i+1); 
      int tot = cerca(start,end);
      printf("Figlio %d, cerco tra %d e %d, trovati %d \n",i,start,end,tot);
      ssize_t e = write(up[1],&tot,sizeof(int));
      if(e!=sizeof(int)) die("Errore scrittura pipe");
      close(up[1]);
      exit(0);
    }  
  }
  int tot=0;
  close(up[1]);
  while(true) {
    int x;
    ssize_t e = read(up[0],&x,sizeof(int));
    if(e==0) break;
    tot += x;
  }
  printf("Numero primi p tali che  %d <= p < %d è: %d\n",n0,n1,tot);
  return 0;
}
