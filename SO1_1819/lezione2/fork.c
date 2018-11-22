#include "xerrori.h"

// primo esempio di generazione di processi mediante fork

// notare l'uso delle funzioni x... che eseguono chiamate di sistema
// o di libreria e verificano la presenza di eventuali errori
// Il sorgente di queste funzioni e' in xerrorsi.c i prototipi in xerrori.h 

int main(int argc, char *argv[])
{ 
  int n=3;  // numero di figli da generare
  for(int i=0;i<n;i++) {
    pid_t p = xfork(__FILE__,__LINE__);
    if(p==0) {// figlio
      printf("Io sono %d figlio %d-esimo di %d, principe di Moria\n",getpid(),i,getppid());
      exit(i);
    }  
  }
  // padre
  printf("Io sono %d figlio di %d, re sotto la montagna\n",getpid(),getppid());
  for(int i=0;i<n;i++) {
    int status;
    pid_t p = xwait(&status,__FILE__,__LINE__);
    printf("E' terminata la missione del figlio %d\n",p);
    if(WIFEXITED(status)) 
      printf("Draghi uccisi da questo figlio: %d\n",WEXITSTATUS(status));
    else
      printf("Figlio non terminato con exit\n");  
  }
  printf("Finito\n");
  return 0;
}







