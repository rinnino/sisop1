#include "xerrors.h"
#include <stdbool.h> // definizione tipo bool

bool primo(int n);

int main(int argc, char const *argv[]) {
  if(argc!=3) {
    printf("Uso\n\t%s M N\n", argv[0]);
    exit(1);
  }

  int nProcessi = atoi(argv[2]);
  int m = atoi(argv[1]);

  //usiamo una pipe come schema di comunicazione
  //padre  <-----()__p__()--- processo figlio
  int p[2];
  int e = pipe(p);
  if(e>0) die("Problema creazione pipe p");

  // ---- creazione processi figli
  int flagFiglio;
  pid_t pid=0;
  for(flagFiglio=0; flagFiglio<nProcessi; flagFiglio++ ){
    pid = xfork(__LINE__, __FILE__);
    if(pid==0){break;} // un figlio non deve creare altri figli ...
  }
  if(pid==0){
    /*
    * CODICE FIGLI
    * Il calcolo deve essere svolto in parallelo da N processi utilizzando il
    * seguente schema. Per i=0,...,N-1 il processo figlio i-esimo deve testare
    * la primalità dei numeri che sono congrui a 2i+1 modulo 2N. Ad esempio se
    * M=50 e N=4 il figlio 0 deve testare gli interi congrui a 1 modulo 8,
    * quindi:
    * il figlio 0 : 1,9,17,25,33,41,49
    * il figlio 1 : 3,11,19,27,35,43
    * il figlio 2 : 5,13,21,29,37,45
    * il figlio 3: 7,15,23,31,39,47
    */

    //chiudiamo il lato della pipe che non ci serve
    close(p[0]); //solo scrittura

    int t; // numero da testare
    int sum=0; //somma primi trovati per questo processo
    t = (2*flagFiglio + 1)%(2*nProcessi);
    while(t<m){
      printf("[f%d]Testo %d\n", flagFiglio, t);
      if(primo(t)){
        printf("[f%d]%d e' primo.\n", flagFiglio, t);
        sum+=1;
      }
      t+=2*nProcessi;
    }
    //scriviamo sulla pipe per inviare la somma parziale al padre
    ssize_t ew;
    ew = write(p[1], &sum, sizeof(int));
    if(ew != sizeof(int)){
      perror("[p]Problema con la write pipe p");
      exit(1);
    }
    //chiudo la pipe
    close(p[1]);

  }else{
    /*
    * CODICE PADRE
    */
    //chiudiamo il lato della pipe che non ci serve
    close(p[1]); //solo lettura

    //raccogliamo e sommiamo le sommatorie parziali dei figli
    ssize_t er;
    int sum=0;
    int n;
    while(true){
      er = read(p[0], &n, sizeof(int));
      if(er == 0){break;} //fine pipe
      if(er != sizeof(int)){
        perror("[p]Problema con la read pipeUp");
        exit(1);
      }
      sum+=n;
    }

    //chiudiamo del tutto la pipe
    close(p[0]);

    printf("[p]I primi dispari minori di %d sono %d.\n", m, sum);

    //attesa figli
    int status;
    int figliChiusi=0;
    for(; figliChiusi<nProcessi; figliChiusi++){
      xwait(&status, __LINE__, __FILE__);
      //printf("[p]figlio terminato\n");
    }
  }
  /*
  * CODICE FINALE PADRE E FIGLI
  */
  return 0;
}

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
