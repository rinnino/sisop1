#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>  
#include <assert.h>   
#include "xerrors.h"

// esercizio per la comunicazione fra processi usando valore ritornato


// restituisce true/false a seconda che n sia primo o composto
bool primo(int n);

int cerca(int a, int b)
{
  int tot = 0;
  for(int i=a;i<=b;i++)
    if(primo(i)) tot++;
  return tot;  
}


int main(int argc, char *argv[])
{
  // leggi input
  if(argc!=3) {
    printf("Uso\n\t%s limite numprocessi\n", argv[0]);
    exit(1);
  }
  int n = atoi(argv[1]);
  int p = atoi(argv[2]);
  assert(n>0 && p>0);

  // crea processi
  int i,a,b,e,tot,status,nprimi;
  pid_t pid;

  // crea i processi assegnandogli intervalli distinti
  for(i=0;i<p;i++) {
    a=1+(n/p)*i;
    b= (i+1==p) ? n : (n/p)*(i+1);
    pid = xfork(__LINE__,__FILE__);
    if (pid==0) {
      // codice dei processi figli
      fprintf(stderr,"F Figlio %d con pid %d cerca fra %d e %d\n",i,getpid(), a,b);
      int tot = cerca(a,b);
      assert(tot<256);     
      exit(tot);
    }
  }

  // codice del processo padre
  // legge i risultati da status
  tot = 0;
  i=0;
  //attendiamo i figli e generiamo la somma dal valore tornato dai figli
  while(i<p) { 
    e = xwait(&status,__LINE__,__FILE__);
    assert(e>0);
    if(WIFEXITED(status) == true){ //se il figlio è terminato normalmente ...
      nprimi = WEXITSTATUS(status); // fetch valore ritornato dal figlio
      printf("P Figlio con pid %d ha trovato %d primi\n",e , nprimi);
      tot += nprimi;
      i++;
    }

  }
  printf("P Il numero di primi minori di %d e' %d\n",n,tot);
}
