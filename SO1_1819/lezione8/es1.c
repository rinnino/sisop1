#include "xerrors.h"

//prototipi
int *random_array(int n);
bool check_sort(int a[], int n);
int somma_array(int a[], int n);
int intcmp(const void *a, const void *b);
void mt_mergesort(int a[], int n, int limite);
void merge(int *a, int m, int *b, int n, int *c);
void *tbody(void *a);
void printIntArray(int *array, int size);

// struct contenente i parametri di input per i thread
typedef struct {
  int *a;
  int aLenght;
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

void mt_mergesort(int a[], int n, int limite)
{
   if(n<=limite) {
     // caso base usa la funzione di libreria qsort
    qsort(a,n,sizeof(int),intcmp);
   }
   //nuovo thread
    else {
      pthread_t t;
      // alloca due array b e c di lunghezza n/2 e n-n/2
      int *b = malloc(sizeof(int)*(n/2));
      int *c = malloc(sizeof(int)*(n-n/2));
      threadInput in;
      in.a = a;
      in.aLenght = n;
      in.b = b;
      in.bLenght = n/2;
      in.limite = limite;

      // in parallelo:

      //creazione thread
      int e = pthread_create(&t, NULL, tbody, (void *) &in);
      assert(e==0);
      //   copia i secondi  n-n/2 elementi di a in c
      memcpy(c, &a[n/2], (n-n/2)*sizeof(int));
      //   invoca mt_mergesort(c,n-n/2,limite)
      mt_mergesort(c, n-n/2, limite);
      // quando entrambi i thread hanno finito
      pthread_join(t, NULL);
      assert(e==0);
      printf("[mergesort]b: ");
      printIntArray(b, n/2);
      printf("[mmergesort]c: ");
      printIntArray(c, n-n/2);
      // esegui il merge di b[] e c[] in a[]
      merge(b, n/2, c, n-n/2, a);
      // dealloca b[] e c[]
      free(b);
      free(c);
    }
}

void merge(int *a, int m, int *b, int n, int *c){
	int pa = 0, pb = 0, pc = 0;
	while(pa < m && pb < n){
		if(a[pa] < b[pb])
			c[pc++] = a[pa++];
		else
			c[pc++] = b[pb++];
	}
	if(pa == m)
		while(pb < n)	//Array A exhausted
			c[pc++] = b[pb++];
	else
		while(pa < m)	//Array B exhausted
			c[pc++] = a[pa++];
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
  mt_mergesort(a,n,limite);
  if(check_sort(a, n))
    fprintf(stderr,"Sort OK\n");
  else
    fprintf(stderr,"Sorting fallito\n");
  fprintf(stderr, "Somma elementi output: %d\n", somma_array(a,n));
  printIntArray(a, n);
  return 0;
}
