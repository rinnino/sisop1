#include "xerrori.h"

int main(int argc, char const *argv[]) {

  /*variabili utili*/
  pid_t pid;
  int e;
  int status;

  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s n\n\n", argv[0]);
    exit(1);
  }

  /*creiamo le due pipe*/
  int pipePari[2], pipeDispari[2];
  e = pipe(pipePari);
  if(e>0) die("Problema creazione pipe");
  e = pipe(pipeDispari);
  if(e>0) die("Problema creazione pipe");

  /*creazione processi figli*/
  int nProc = 0;
  for(; nProc < 2; nProc++){
    pid = xfork(__FILE__, __LINE__);
    if(pid == 0){
      break;
    }
  }

  if(pid == 0){
    /*
     * Codice processi figli
     */
     if(nProc == 0){
       /* primo figlio, pari*/
       /* sistemiamo le pipe*/
       //pipe solo in scrittura
       close(pipePari[1]); // solo lettura
       close(pipeDispari[0]);
       close(pipeDispari[1]); // la pipe dei dispari non ci serve

       /* creiamo un file in scrittura */
       FILE *f = xfopen(strcat(argv[1], ".pari"),"w+",__FILE__,__LINE__);
       assert(f!=NULL);

       /* leggiamo dalla pipe e scriviamo*/
       int fromPipe;
       size_t ew;
       ssize_t er;
       while(true){
         er = read(pipePari[0], &fromPipe, sizeof(int));
         if(er == 0){
           break; // fine pipe
         }
         if(er!= sizeof(int)){
           //errore lettura
           die("[fp]Problema read pipe\n");
         }
         ew = fwrite(&fromPipe, sizeof(int), 1, f);
         if(ew != 1){
           die("[fp]Problema write\n");
         }
       }
       /*chiudi file*/
       if(fclose(f) != 0){
         die("[fp]Problema close\n");
       }
     }
     if(nProc ==1){
       /* secondo figlio, dispari*/
       /* sistemiamo le pipe*/
       //pipe solo in scrittura
       close(pipeDispari[1]); // solo lettura
       close(pipePari[0]);
       close(pipePari[1]); // la pipe dei pari non ci serve

       /* creiamo un file in scrittura */
       FILE *f = xfopen(strcat(argv[1], ".dispari"),"w+",__FILE__,__LINE__);
       assert(f!=NULL);

       /* leggiamo dalla pipe e scriviamo*/
       int fromPipe;
       size_t ew;
       ssize_t er;
       while(true){
         er = read(pipeDispari[0], &fromPipe, sizeof(int));
         if(er == 0){
           break; // fine pipe
         }
         if(er!= sizeof(int)){
           //errore lettura
           die("[fd]Problema read pipe");
         }
         ew = fwrite(&fromPipe, sizeof(int), 1, f);
         if(ew != 1){
           die("[fd]Problema write");
         }
       }
       /*chiudi file*/
       if(fclose(f) != 0){
         die("[fp]Problema close\n");
       }
     }

  }else{
    /*
     * Codice processo padre
     */

     /* apriamo file in lettura come stream */
     FILE *f = xfopen(argv[1], "r", __FILE__, __LINE__);
     if(f == NULL){
       perror("Problema con l'apertura del file");
     }

    //pipe solo in scrittura
    close(pipePari[0]);
    close(pipeDispari[0]);

    //scorriamo i numeri riga per riga ed inviamo alla giusta pipe
    char *linea = NULL;
    int n;
    while(getline(linea, 0, f) != -1){
      n = atoi(linea);
      if(n%2 == 0){
        //invio a pipe pipePari
        ssize_t ew = write(pipePari[1], &n, sizeof(int));
        if(ew != sizeof(int)){
          perror("Problema con la write pipePari");
          exit(1);
        }
      }else{
        //invio a pipe pipeDispari
        ssize_t ew = write(pipeDispari[1], &n, sizeof(int));
        if(ew != sizeof(int)){
          perror("Problema con la write pipeDispari");
          exit(1);
        }
      }
      free(linea); // getline() alloca con malloc()
    }
    //chiudiamo le pipe (equivale a mandare eof ai consumatori)
    close(pipePari[1]);
    close(pipeDispari[1]);

    /*attesa figli*/
    printf("[P]Attendo i processi figli\n");

    e = xwait(&status, __FILE__, __LINE__);
    for(int i = 0; i < 2; i++){
      if(WIFEXITED(status) == true){
        //nulla di che
      }
      else{
        fprintf(stderr, "[P]Attenzione figlio uscito con errore!\n");
      }
    }

    /*chiusura strutture dati*/
    if(fclose(f) != 0){
      perror("Problema chiusura file.");
    }

    printf("[P]Procedura terminata.\n");
  }















  return 0;
}
