#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "xerrors.h"

// prototipi di funzione
int *random_array(int n);
int intcmp(const void *a, const void *b);
void array_sort(int *a, int n);
bool check_sort(int *a, int n);

// main
int main(int argc, char **argv)
{
    int numElementi;

    //inizializzazione rand
    srand(time(NULL));

    if(argc!= 2)
    {
        fprintf(stderr,"Uso: nomeProgramma n\n");
        exit(1);
    }
    int n= atoi(argv[1]);
    assert(n>0); //controllo numero passato da argv[1]

    //creazione area memoria condivisa
    int e, shmid;
    //identificatore pid
    pid_t pid;

    // creazione array di memoria condivisa in lettura scrittura
    shmid = shmget(IPC_PRIVATE,n*sizeof(int),0600);
    if(shmid == -1) { perror("shared memory allocation"); exit(1);}
    //otteniamo puntatore are condivisa da id
    int *memoriaCondivisa = shmat(shmid,NULL,0);
    // visualizza pid per controllo memoria condivisa
    printf("[Padre] Il pid di questo processo e': %d, l'id della memoria: %d\n",getpid(),shmid);
    // creazione array random
    memcpy(memoriaCondivisa, random_array(n), n*sizeof(int));

    //stampa debug
    printf("[Padre] la memoria condivisa contiene:");
    for(int i=0; i<n; i++){
        printf(" [%d]", memoriaCondivisa[i]);
    }
    printf("\n");

    //fork
    pid= xfork(__LINE__, __FILE__);
    if(pid==0){
        /*
         * Codice processo figlio
         */

        // visualizza pid per controllo memoria condivisa
        printf("[Figlio] Il pid di questo processo e': %d, l'id della memoria: %d\n",getpid(),shmid);

        //calcolo ultimo elemento
        numElementi = n-((n/2)+1);

        //printf debug
        if(numElementi>0){
            printf("[Figlio] ho in carico l'array da [%d] a [%d], %d elementi\n", (n/2)+1, n-1, numElementi);
        }else{
            printf("[Figlio] non ho array in carico!\n");
        }

        //ordinamento da (n/2)+1 ad n-1
        array_sort(memoriaCondivisa+(1+(n/2)), numElementi);

        // detach memoria condivisa  
        e = shmdt(memoriaCondivisa);
        if(e<0) {perror("shmdt"); exit(1);}
        exit(0);
       
    }else{
        /*
         * Codice processo padre
         */

        //ordinamento da 0 ad ad n/2
        array_sort(memoriaCondivisa, (n/2)+1);
        //stampa debug
        printf("[Padre] ho in carico l'array da [0] a [%d]\n", (n/2));

        //attendo terminazione figlio
        pid  = xwait(NULL,__LINE__,__FILE__);

        //stampa debug array
        printf("[Padre] Array ordinato:");
        for(int i=0; i<n; i++){
            printf(" [%d]", memoriaCondivisa[i]);
        }
        printf("\n");

        //check ordine
        //if(check_sort(memoriaCondivisa, n)){
        //    printf("[Padre] Array condiviso ordinato.");
        //}else{
        //    printf("[Padre] Array condiviso NON ordinato!");
        //}

        // detach e rimozione memoria condivisa
        e = shmdt(memoriaCondivisa);
        if(e<0) {perror("shmdt"); exit(1);}
        e = shmctl(shmid, IPC_RMID,  NULL);
        if(e == -1) {perror("shmctl"); exit(1);}
        return 0;

    }



}

// genera array di n elementi con interi random tra 0 e RAND_MAX 
int *random_array(int n)
{
  assert(n>0);
  int *a = malloc(n* sizeof(int));
  assert(a!=NULL);
  for(int i=0;i < n;i++)
    a[i] = (int) rand();
  return a;
}

// funzione di confronto tra interi passata da array_sort a qsort
int intcmp(const void *a, const void *b)
{
  return *((int *) a) - *((int *) b);
}
// esegue il sort dell'array a[0..n] utilizzando la funzione di libreria qsort
void array_sort(int *a, int n)
{
  qsort(a,n,sizeof(int), intcmp);
}

// verifica che array a[0..n] sia ordinato 
bool check_sort(int *a, int n)
{
  for(int i=0; i < n-1; i++)
     if(a[i] > a[i+1]) return false;
  return true;
}