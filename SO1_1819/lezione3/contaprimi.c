#include <stdio.h>    // permette di usare scanf printf
#include <stdlib.h>   // conversioni stringa/numero rand() abs() exit() etc ...
#include <stdbool.h>  // gestisce tipo bool (per variabili booleane)
#include <assert.h>   // permette di usare la funzione assert

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
  if(argc!=3) {
    printf("Uso:\n\t%s n1 n2\n",argv[0]);
    exit(1);
  }
  int n0 = atoi(argv[1]);
  int n1 = atoi(argv[2]);
  assert(n0>0 && n1>=n0);
  int tot=cerca(n0,n1);
  printf("Numero primi p tali che  %d <= p < %d è: %d\n",n0,n1,tot);
  return 0;
}
