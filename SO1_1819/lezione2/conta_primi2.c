#include "xerrori.h"

bool primo(int n){
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

int main(int argc, char const *argv[]) {
  /* variabili */
  int sum = 0;

  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s n\n\n", argv[0]);
    exit(1);
  }

  int n = atoi(argv[1]);

  /*una volta ottenuto il buffer coi numeri dividiamo il lavoro*/
  pid_t pid;

  pid = xfork(__FILE__, __LINE__);
  if(pid == 0){
    /*codice figlio n/2+1 a n*/
    for(int i = (n/2)+1; i <= n; i++){
      if(primo(i) == true){
        sum++;
      }
    }
    assert(sum<256);
    exit(sum);
  }
  /*codice padre da 0 ad n/2*/
  for(int i = 1; i <= (n/2); i++){
    if(primo(i) == true){
      sum++;
    }
  }

  int status;
  int sumFiglio=0;
  int e = xwait(&status, __FILE__, __LINE__);
  assert(e>0);
  if(WIFEXITED(status) == true){ //se il figlio è terminato normalmente ...
    sumFiglio = WEXITSTATUS(status); // fetch valore ritornato dal figlio
  }

  printf("numeri primi fino a %d: %d\n", n, sumFiglio + sum);

  return 0;
}
