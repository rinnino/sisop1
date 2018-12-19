#include "xerrors.h"
#include <time.h>

// prototipi di funzione
int *random_array(int n);
int massimo(int *a, int n, int p);
void *tbody(void *a);

// struct contenente i parametri di input e output di ogni thread
typedef struct {
  int *numeri; //puntatore da dove iniziare
  int n; //numero di elementi da processare
  int max; // output
  int n_thread;//label identificativo thread
} dati;

// main
int main(int argc, char **argv)
{
  if(argc!= 3)
  {
    fprintf(stderr,"Uso:\n\t %s n p\n",argv[0]);
    exit(1);
  }
  int n= atoi(argv[1]);
  assert(n > 0);
  int p = atoi(argv[2]);
  assert(p > 0 && p <= n);

  int *a = random_array( n );
  int max = a[0];
  for(int i=1; i < n ;i++)
    if(a[i]>max) max=a[i];

  //stampa debug
  printf("[padre] Array:");
  for(int i=0; i<n; i++){
      printf(" [%d]", a[i]);
  }
  printf("\n");

  printf("Massimo calcolato con un thread: %d\n",max);

  // questa e' la funzione da scrivere per esercizio
  max = massimo(a,n,p);
  printf("Massimo calcolato con %d thread: %d\n",p, max);

  free(a);
  return 0;
}

// calcola il massimo dell'array a[] di n elementi
// utilizzando p thread
// ogni thread deve leggere al piu' (n/p)+1 elementi
// p : 0 > p >= n, massimo() NON verifica la condizione.
int massimo(int *a, int n, int p) {
    int e;
    //variabili per i thread
    pthread_t t[p];
    //argomenti per le thread
    dati argomenti[p];

    // creazione di p thread con il loro range di numeri
    //num determina il numero di thread che avranno (n/p)+1 numeri
    //base tiene conto della posizione di partenza dell array per
    //un certo thread. il primo thread avrà base 0, il secondo
    // 0 + "quantità numeri thread 0", e cosi via ...
    for(int i=0,num=n%p,base=0; i<p; i++){
        //prepariamo gli argomenti
        argomenti[i].n_thread = i; //identificativo

        /*
        * esempio:
        * n=20 p=3 n/p=6 n%p=2 quindi le aree saranno:
        * da 0 a 7   (n/p +1 elementi)
        * da 7 a 14  (n/p +1 elementi)
        * da 14 a 20 (n/p elementi)
        *
        * i thread con "n/p+1 elementi" sono esattamente n%p
        *
        */

        if(num>0){ //se dobbiamo assegnare al thread n/p+1 elementi
            argomenti[i].n = (n/p)+1;
            num--;

        }else{ //oppure n/p elementi
            argomenti[i].n = n/p;
        }

        //determiniamo puntatore primo elemento
        argomenti[i].numeri = a+base; //aritmetica puntatori

        //ed aggiorniamo la variabile base per sapere dove partire
        //col prossimo thread
        base += argomenti[i].n;

        //creazione thread
        e = pthread_create(&t[i], NULL, tbody, (void *) &argomenti[i]);
        assert(e==0);
    }

    //variabile per valore massimo
    int max=0;

    //attesa thread e determinazione del massimo
    for(int i=0; i<p; i++){
        e = pthread_join(t[i], NULL);
        if(e==0) {
            //fprintf(stderr,"Thread %d terminato\n",i);
            if(argomenti[i].max > max){
                max = argomenti[i].max;
            }
        }else{
            fprintf(stderr,"Errore join %d\n",e);
        }

    }
    return max;
}

// genera array di n elementi con interi random tra -100000 e 100000
int *random_array(int n)
{
  assert(n>0);
  int *a = malloc(n* sizeof(int));
  assert(a!=NULL);
  srand(time(NULL));
  for(int i=0; i < n ;i++)
    a[i] = (random()%200001) - 100000;
  return a;
}

// funzione eseguita dal thread
void *tbody(void *a)
{
    dati *arg = (dati *)a; //cast puntatore struttura dati
    fprintf(stderr,"Thread %d cerca %d elementi a partire da %d \n", arg->n_thread, arg->n, arg->numeri[0]) ;
    arg->max = arg->numeri[0];
    for(int i=1; i<arg->n; i++){
        //printf("thread %d numero %d\n", arg->n_thread, arg->numeri[i]);
        if(arg->numeri[i]>arg->max){
            arg->max = arg->numeri[i];
        }
    }
    pthread_exit(NULL);
}
