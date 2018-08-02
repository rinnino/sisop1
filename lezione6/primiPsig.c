#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <assert.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "xerrors.h"
#include <stdbool.h>

/*
* Attenzione, validita solo didattica.
* i segnali non sono uno strumento da usare per la sincronizzazione:
* probabilmente non è possibile, con le sole tecniche viste a lezione,
* realizzare un meccanismo che funzioni il 100% delle volte
* (vi sarete già accorti che a causa di differenze di scheduling lo
* stesso programma eseguito due volte si può comportare in maniera
* diversa). Provare a trovare una soluzione però è un ottimo modo per
* imparare.
*
* questo programma funziona 1 volta si e 10 no.
* per lo piu delle volte rimane bloccato su una wait per sempre ...
* per colpa delle differenze di schedulazione
* con una sleep probabilmente si puo apporre un fix però non è molto
* efficente ....
* la versione primiPsigSleep.c prova a fixare ...
*/

//prototipi
bool primo(int n);
void handler(int s);

// main
int main(int argc, char **argv)
{
    int numFiglio=0; //numero del figlio al momento del fork
    int sum=0; //sommatoria finale primi

    // definisce signal handler 
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);     // setta a "insieme vuoto" sa.sa_mask maschera di segnali da bloccare 
    sa.sa_flags = SA_RESTART;     // restart system calls  if interrupted
    sigaction(SIGUSR1,&sa,NULL);  // handler per USR1

    if(argc!= 3)
    {
        fprintf(stderr,"Uso: nomeProgramma m n\nm: numeri dispari minori di m\nn: numero processi\n");
        exit(1);
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    assert(m>0); //controllo numero passato da argv[1]
    assert(n>1); //almeno 2 processi

    //creazione area memoria condivisa
    int e, shmid;
    //identificatore pid
    pid_t pid=0;
    pid_t pidPadre;

    //memorizzo pid padre
    pidPadre = getpid();

    // creazione array di memoria condivisa in lettura scrittura
    shmid = shmget(IPC_PRIVATE,n*sizeof(int),0600);
    if(shmid == -1) { perror("shared memory allocation"); exit(1);}
    //otteniamo puntatore are condivisa da id
    int *memoriaCondivisa = shmat(shmid,NULL,0);
    // visualizza pid per controllo memoria condivisa
    printf("[Padre] Il pid di questo processo e': %d, l'id della memoria: %d\n",getpid(),shmid);

    //per sicurezza scrivo tutti zero nell'array condiviso
    memset(memoriaCondivisa, 0, n*sizeof(int));

    //fork
    for(int i=0; i<n; i++){
        numFiglio = i;
        pid = xfork(__LINE__, __FILE__);
        if(pid==0) break;
    }

    //a questo punto abbiamo n figli

    if(pid==0){
        /*
         * Codice processo figlio
         */
        
        //stampa debug
        printf("[figlio%d] figlio %d creato\n", numFiglio, numFiglio);

        for(int j=2*numFiglio+1; j<m; j+= 2*n){
            //stampa debug
            printf("[figlio%d] verifico %d\n", numFiglio, j);

            if(primo(j)){
                //j primo, incrementiamo contatore nella mem condivisa
                memoriaCondivisa[numFiglio]++;
            }
        }

        //devo inviare un segnale al padre per risvegliarlo
        kill(pidPadre, 10); //10 = sigusr1

        // detach memoria condivisa  
        e = shmdt(memoriaCondivisa);
        if(e<0) {perror("shmdt"); exit(1);}
        exit(0);

    }else{
        /*
         * Codice processo padre
         */

        //attendiamo la terminazione degli n figli
        for(int i=0; i<n; i++){
            //attendo terminazione figlio con segnale
            pause();
        }

        //stampa debug
        printf("[Padre] i figli han terminato il loro lavoro, procedo con la somma ...\n");

        //stampa debug e sommatoria
        printf("[Padre] valore area condivisa:");
        for(int i=0; i<n; i++){
            printf(" [%d]", memoriaCondivisa[i]);
            sum += memoriaCondivisa[i];
        }
        printf("\n");
        printf("[Padre]: ci sono %d numeri primi dispari minori di %d\n", sum, m);

        // detach e rimozione memoria condivisa
        e = shmdt(memoriaCondivisa);
        if(e<0) {perror("shmdt"); exit(1);}
        e = shmctl(shmid, IPC_RMID,  NULL);
        if(e == -1) {perror("shmctl"); exit(1);}
        return 0;
    }
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

// funzione che viene invocata quando viene ricevuto un segnale USR1 
void handler(int s)
{
    if(s==SIGUSR1){
        //aspetto questo segnale ma senza far nulla
        printf("il processo %d ha ricevuto il segnale %d\n", getpid(), s);
    }
}