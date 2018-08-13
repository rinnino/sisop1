/*
* versione dell'esercizio fatta con fra
* logica per determinare gli intervalli migliore
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

// prototipi di funzione
int *random_array(int n);
int massimo(int *a, int n, int p);


typedef struct{
    
    int *inizio;    //puntatore inizio
    int n;          //numero elementi da processare
    int max;        //output
    int label;  //etichetta numero thread
}dati;




// main
int main(int argc, char **argv)
{
    
    srand(time(NULL));
  if(argc!= 3)
  {
    fprintf(stderr,"Uso:\n\t %s n p\n",argv[0]);
    exit(1);
  }
  int n= atoi(argv[1]);
  assert(n > 0);
  int p = atoi(argv[2]);
  assert(p > 0);

  int *a = random_array(n);
  
  for(int i=0;i < n; i++) printf("[ %d ] ", a[i]);
  printf("\n\n");
  int max = a[0];
  for(int i=1; i < n ;i++)
    if(a[i]>max) max=a[i];
      
  printf("Massimo calcolato con un thread: %d\n",max); 
  
  // questa e' la funzione da scrivere per esercizio
  max = massimo(a,n,p);
  printf("Massimo calcolato con %d thread: %d\n",p, max); 

  free(a);
  return 0;
}


void *tbody(void *a)
{
   dati *arg = (dati *)a;
   fprintf(stderr,"Thread %d cerca %d elementi da %d a %d\n", arg->label, arg->n, arg->inizio[0], *((arg->inizio)+arg->n-1));
   arg->max = arg->inizio[0];
   for(int i=1;i<arg->n;i++){
       if(arg->inizio[i] > arg->max) {
           arg->max = arg->inizio[i];}
       
}

  pthread_exit(NULL);    // non restituisce nulla attraverso la exit
}


// calcola il massimo dell'array a[] di n elementi
// utilizzando p thread
// ogni thread deve leggere al piu' (n/p)+1 elementi 
int massimo(int *a, int n, int p) {
    
    int i,e,max;
    max=0;
    
    pthread_t t[p];
    
    //argomenti per i thread
    dati argomenti[p];
    for(i = 0; i < p; i++){
        
        argomenti[i].label = i;
        argomenti[i].inizio = ((i==p-1) || (i==0)) ? a+(i*(n/p))+n%p : a+(i*(n/p))+1 ;
        argomenti[i].n = i==p-1 ? n/p : (n/p)+1;
        
        
        
        pthread_create(&t[i], NULL, tbody, (void *) &argomenti[i]);
        
    }
    
    max = argomenti[0].max; 
    
    for(i=0;i<p;i++){
        
        e = pthread_join(t[i], NULL);
        if(e==0) fprintf(stderr,"Thread %d terminato!!!\n",i);
    
    }
    
    for(i=1; i<p; i++){
        
        if(argomenti[i].max > max)  max = argomenti[i].max;
        
    }
    
    
    return max;
}

// genera array di n elementi con interi random tra -100000 e 100000
int *random_array(int n)
{
  assert(n>0);
  int *a = malloc(n* sizeof(int));
  assert(a!=NULL);
  for(int i=0; i < n ;i++)
    a[i] = (random()%200001) - 100000;
  return a;
}
