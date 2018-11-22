#include "xerrori.h"


void scrivi1n(int n, char *nome) 
{
  // apre file in scrittura
  int fd = open(nome,O_WRONLY|O_CREAT|O_TRUNC,0666);
  if(fd<0) die("Errore creazione file");
  // scrive valori sul file
  for(int i=1;i<=n;i++) {
    ssize_t e = write(fd,&i, sizeof(int)); // scrive su f il contenuto di i 
    if(e!=sizeof(int)) {
      perror("Errore scrittura file");
      exit(1);
    }
  }
  // chiude file 
  int e = close(fd);
  if(e!=0) {
    perror("Errore chiusura file");
    exit(1);
  }
}


int main(int argc, char *argv[])
{
  if(argc!=3) {
    fprintf(stderr, "Uso:\n\t%s n nomefile\n\n",argv[0]);
    exit(1);
  }
  int n = atoi(argv[1]);
  if(n<=0) {
    fprintf(stderr, "Inserisci un intero positivo\n");
    exit(1);
  }
  scrivi1n(n,argv[2]);
}
    
    
    
