// genera array di n elementi con interi random tra 0 e 1999
int *random_array(int n)
{
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
    somma + a[i];
  return somma;
}

// funzione di confronto tra gli interi puntati da a e b
// da usare con la funzione di libreria qsort
int intcmp(const void *a, const void *b)
{
  return *((int *) a) - *((int *) b);
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

void mt_mergesort(int a[], int n, int limite)
{
   if(n<=limite) {
     // caso base usa la funzione di libreria qsort
    qsort(a,n,sizeof(int),intcmp);
   }
    else {
      // alloca due array b e c di lunghezza n/2 e n-n/2
      // in parallelo:
      //   copia i primi n/2 elementi di a in b
      //   invoca mt_mergesort(b,n/2,limite)
      //   copia i secondi  n-n/2 elementi di a in c
      //   invoca mt_mergesort(c,n-n/2,limite)
      // quando entrambi i thread hanno finito
      // esegui il merge di b[] e c[] in a[]
      // dealloca b[] e c[]
    }
}

int main(int argc, char *argv[])
{
  int n, limite;

  if((argc<2)||(argc>3)) {
    fprintf(stderr,"USO:  %s numel limite\n\n",argv[0]);
    exit(1);
  }
  n = atoi(argv[1]);
  limite = atoi(argv[2])
  if(n<1 || limite <1) {
    fprintf(stderr,"Input non valido (main)\n");
    exit(1);
  }
  int a* = random_array( n );
  fprintf(stderr, "Somma elementi input: %d\n", somma_array(a,n));
  mt_mergesort(a,n,limite);
  if(check_sort(array,n))
    fprintf(stderr,"Sort OK\n");
  else
    fprintf(stderr,"Sorting fallito\n");
  fprintf(stderr, "Somma elementi output: %d\n", somma_array(a,n));
  return 0;
}
