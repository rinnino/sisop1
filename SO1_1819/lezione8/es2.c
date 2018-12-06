#include "xerrors.h"

//prototipi
int *random_array(int n);
bool check_sort(int a[], int n);
int somma_array(int a[], int n);
int intcmp(const void *a, const void *b);
void *tbody(void *a);
void printIntArray(int *array, int size);
void mt_qsort(int a[], int p, int r, int limite);
int partition(int A[], int p, int r);

// struct contenente i parametri di input per i thread
typedef struct {
  int *a;
  int aLenght;void mt_qsort(int a[], int p, int r, int limite)
  int *b;
  int bLenght;
  int limite;
} threadInput;

/* stampa array interi */
void printIntArray(int *array, int size){
  for(int i = 0; i < size; i++){
    printf("[%d]", array[i]);
  }
  printf("\n");
}

// genera array di n elementi con interi random tra 0 e 1999
int *random_array(int n)
{
  srand(time(NULL));
  assert(n>0);
  int *a = malloc(n* sizeof(int));
  if(a==NULL) die("errore allocazione");
  for(int i=0;i < n;i++)
    a[i] = (int) random() % 2000;
  return a;
}

// verifica che array a[0..n-1] sia ordinato
bool check_sort(int a[], int n)
{
  for(int i=0; i < n-1; i++)
     if(a[i] > a[i+1]) return false;
  return true;
}

// calcola somma elementi array
int somma_array(int a[], int n)
{
  int somma=0;
  for(int i=0;i < n;i++)
    somma += a[i];
  return somma;
}

// funzione di confronto tra gli interi puntati da a e b
// da usare con la funzione di libreria qsort
int intcmp(const void *a, const void *b)
{
  return *((int *) a) - *((int *) b);
}

// ordina gli elementi a[p] ... a[r]
void mt_qsort(int a[], int p, int r, int limite)
{
   if(r-p+1<=limite) {
     // caso base usa la funzione di libreria qsort
     qsort(a...);
    }
    else {
      // invoca la procedura partition per mettere in
      // a[p] ... a[q] gli elementi "piccoli" e in a[q+1]...a[r] gli elementi "grandi"
      // in parallelo
      //    invoca mt_qsort(a,p,q,limite)
      //    invoca mt_qsort(a,q+1,r,limite)
    }
}

/* ***********************************************************
   partition con scelta pivot mediano di 3
   per evitare l'uso di random che e' lenta e non completamente thread safe
   *********************************************************** */
int partition(int A[], int p, int r)
{
  int t,x;
  int i,j;

  // il pivot sara' il mediano tra A[p],A[i],A[r]
  i = (p+r)/2;
  if( A[p] > A[i] )
      {t=A[i];A[i]=A[p];A[p]=t;} // scambia A[i]<->A[p]
  if( A[p] > A[r] )
      {t=A[r];A[r]=A[p];A[p]=t;} // scambia A[r]<->A[p], ora A[p] e' il minino
  if( A[i] > A[r] )
      {t=A[r];A[r]=A[i];A[i]=t;} // scambia A[r]<->A[i], ora A[p]<= A[i] <= A[r]

  x = A[i];                   // pivot
  t=A[i];A[i]=A[p];A[p]=t;    // scambia A[i]<->A[p]

  i= p-1; j= r+1;
  while(1) {
    while(A[--j]>x) ; // scan da destra a sinistra: esce se A[j]<=x
    while(A[++i]<x) ; // scan da sinistra a destra: esce se A[i]>=x
    if(i<j) {
      t=A[i]; A[i]=A[j]; A[j]=t;  // scambia A[i] <-> A[j] e riprendi scansioni
    }
    else break;       // se i>=j termina
  }
  return j;
}


// funzione eseguita dal thread
void *tbody(void *a)
{
  threadInput *arg = (threadInput *)a; //cast puntatore struttura dati
  //   copia i primi n/2 elementi di a in b
  memcpy(arg->b, arg->a, (arg->bLenght)*sizeof(int));
  //   invoca mt_mergesort(b,n/2,limite)
  mt_mergesort(arg->b, arg->bLenght, arg->limite);

  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  int n, limite;

  if((argc<2)||(argc>3)) {
    fprintf(stderr,"USO:  %s numel limite\n\n",argv[0]);
    exit(1);
  }
  n = atoi(argv[1]);
  limite = atoi(argv[2]);
  if(n<1 || limite <1) {
    fprintf(stderr,"Input non valido (main)\n");
    exit(1);
  }
  int *a = random_array(n);
  printIntArray(a, n);
  fprintf(stderr, "Somma elementi input: %d\n", somma_array(a,n));
  mt_qsort(a,0,n-1,limite);
  if(check_sort(a, n))
    fprintf(stderr,"Sort OK\n");
  else
    fprintf(stderr,"Sorting fallito\n");
  fprintf(stderr, "Somma elementi output: %d\n", somma_array(a,n));
  printIntArray(a, n);
  return 0;
}
