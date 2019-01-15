/*
* esame del 24/01/17, parte sui processi.
*/

/*
*   schema comunicazione
*
*   padre  ---(pipeDown)---> processo figlio
*   padre  <-----(pipeUp)--- processo figlio
*
*/

#include "xerrors.h"

#define Buf_size 10


int main(int argc, char const *argv[]) {
  //controllo input
  if(argc<3) {
    printf("Uso\n\t%s N nomefile\n", argv[0]);
    exit(1);
  }
  int nProcessi = atoi(argv[1]);
  assert(nProcessi>0);

  //creiamo le due pipe
  int pipeUp[2], pipeDown[2];
  int e = pipe(pipeUp);
  if(e>0) die("Problema creazione pipeUp");
  e = pipe(pipeDown);
  if(e>0) die("Problema creazione pipeDown");

  // ---- creazione processi figli
  int flagFiglio;
  pid_t pid;
  for(flagFiglio=0; flagFiglio<nProcessi; flagFiglio++ ){
    pid = xfork(__LINE__, __FILE__);
    if(pid==0){break;} // un figlio non deve creare altri figli ...
  }
  if(pid==0){
    /*
    * CODICE FIGLI
    */

    //debug
    printf("[f%d]Processo Iniziato\n", flagFiglio);

    //chiudiamo i lati delle pipe che non ci servono
    close(pipeDown[1]); //solo lettura
    close(pipeUp[0]); //solo scrittura


    int sum=0;
    int fromPipe;
    ssize_t er;
    while(true){
      er = read(pipeDown[0], &fromPipe, sizeof(int));
      if(er == 0){break;} //fine pipe
      if(er!= sizeof(int)){
        //errore lettura
        die("[fd]Problema read pipeDown");
      }
      //debug
      printf("[f%d]Leggo %d da pipeDown\n", flagFiglio, fromPipe);
      sum+=(fromPipe*fromPipe);
    }
    close(pipeDown[0]);

    //mandiamo la somma al padre
    ssize_t ew = write(pipeUp[1], &sum, sizeof(int));
    if(ew != sizeof(int)){
      perror("[fd]Problema con la write pipeUP");
      exit(1);
    }
    close(pipeUp[1]);
  }
  else{
    /*
    * CODICE PADRE
    */

    //chiudiamo i lati delle pipe che non ci servono
    close(pipeDown[0]); //solo scrittura
    close(pipeUp[1]); //solo lettura

    //leggo da file ed invio i figli tramite pipeDown
    int n;
    ssize_t ew;
    FILE *f = xfopen(argv[2], "r", __LINE__,__FILE__);
    while(true){
      e = fscanf(f, "%d", &n);
      if(e!=1){break;} // se il file non ha piu numeri esci
      //scrivi il numero sulla pipeDown
      //printf("[p]Provo a scrivere %d su pipeDown\n", n);
      ew = write(pipeDown[1], &n, sizeof(int));
      if(ew != sizeof(int)){
        perror("[p]Problema con la write pipeDown");
        exit(1);
      }
    }
    //posso chiudere il file e pipeDown
    close(pipeDown[1]);
    fclose(f);

    //ora devo leggere da pipeUp e fare la somma
    ssize_t er;
    int sum=0;
    while(true){
      er = read(pipeUp[0], &n, sizeof(int));
      if(er == 0){break;} //fine pipe
      if(er != sizeof(int)){
        perror("[p]Problema con la read pipeUp");
        exit(1);
      }
      sum+=n;
    }
    close(pipeUp[0]);

    printf("[p]La sommatoria è: %d\n", sum);

    // ---- attesa figli
    printf("[p]Procedura terminata, attendo i figli.\n");
    int status;
    int figliChiusi=0;
    for(; figliChiusi<nProcessi; figliChiusi++){
      xwait(&status, __LINE__, __FILE__);
      //printf("[p]figlio terminato\n");
    }
    printf("[p]Ho terminato i %d figli\n", figliChiusi);
  }
  return 0;
}
