#include "xerrors.h"
#include <time.h>
#define NAME "/l6e1shm"

//---- prototipi

//ritorna la somma dei numeri di un array di interi lungo n
long sumIntArray(int *array, int n);
// verifica che array a[0..n] sia ordinato
bool check_sort(int *a, int n);

// popola un array di n elementi con interi random tra 0 e 1999
void random_array(int *array, int n);

// stampa array interi
void printIntArray(int *array, int size);

// funzione di confronto tra gli interi puntati da a e b
int intcmp(const void *a, const void *b);

//conta gli interi minori ad una certa soglia in un array
int contaInfSoglia(int *array, int soglia, int size);

//conta gli interi superiori ad una certa soglia in un array
int contaSupSoglia(int *array, int soglia, int size);

/*sistema tutti i numeri < soglia sulla parte sinistra dell'array senza ordinarli*/
void preordine(int *array, int size, int soglia);

//---- main
int main(int argc, char const *argv[]) {

  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s n\n\n", argv[0]);
    exit(1);
  }

  int n = atoi(argv[1]);
  assert(n>0);
  // ---- creazione array memoria condivisa
  int shm_size = n*sizeof(int); // numero di byte necessari
  int fd = xshm_open(NAME, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *array = simple_mmap(shm_size, fd, __LINE__,__FILE__);
  close(fd); // dopo mmap e' possibile chiudere il file descriptor

  /* popola array con n numeri casuali tra 0 e 1999*/
  random_array(array, n);

  /*debug*/
  printf("[p]array casuale: ");
  printIntArray(array, n);

  /* contiamo quanti min 1000 ci sono*/
  int nInfSoglia = contaInfSoglia(array, 1000, n);

  /*preordine: sposto i numeri < 1000 nella porzione di sinistra dell vettore */
  preordine(array, n, 1000);

  /*creazione figlio*/
  pid_t pid = xfork(__LINE__, __FILE__);

  if(pid==0){
    //---- codice figlio

    //ordino numeri maggiori 1000
    qsort(&array[nInfSoglia], n-nInfSoglia, sizeof(int), intcmp);

    //terminazione figlio
    exit(0);
  }
  //---- codice padre

  //ordino numeri minori 1000
  qsort(array, nInfSoglia, sizeof(int), intcmp);

  //attendo figlio
  int status;
  xwait(&status, __LINE__, __FILE__);

  //stampo risultato
  printf("[p]Array ordinato: ");
  printIntArray(array, n);

  return 0;
}

//ritorna la somma dei numeri di un array di interi lungo n
long sumIntArray(int *array, int n){
  int sum = 0;
  for(int i = 0; i < n; i++){
    sum += array[i];
  }
  return sum;
}

// verifica che array a[0..n] sia ordinato
bool check_sort(int *a, int n)
{
  for(int i=0; i < n-1; i++)
     if(a[i] > a[i+1]) return false;
  return true;
}

// popola un array di n elementi con interi random tra 0 e 1999
void random_array(int *array, int n)
{
  assert(n>0);
  srand(time(NULL));
  assert(array!=NULL);
  for(int i=0;i < n;i++){
    array[i] = (int) random() % 2000;
  }
}

/* stampa array interi */
void printIntArray(int *array, int size){
  for(int i = 0; i < size; i++){
    printf("[%d]", array[i]);
  }
  printf("\n");
}

// funzione di confronto tra gli interi puntati da a e b
int intcmp(const void *a, const void *b)
{
  return *((int *) a) - *((int *) b);
}

int contaInfSoglia(int *array, int soglia, int size){
  if(array == NULL){die("Problema NULL contaInfSoglia()");}
  int ris=0;
  for(int i = 0; i < size; i++){
    if(array[i] < soglia){ris++;}
  }
  return ris;
}

int contaSupSoglia(int *array, int soglia, int size){
  if(array == NULL){die("Problema NULL contaSupSoglia()");}
  int ris=0;
  for(int i = 0; i < size; i++){
    if(array[i] >= soglia){ris++;}
  }
  return ris;
}

/*sistema tutti i numeri < soglia sulla parte sinistra dell'array senza ordinarli*/
void preordine(int *array, int size, int soglia){
  int minSup[size];
  int nInfSoglia = contaInfSoglia(array, soglia, size);
  int *minori = minSup;
  int *superiori = &minSup[nInfSoglia];
  for(int i=0, contMin=0, contSup=0; i<size; i++){
    if(array[i]<soglia){
      minori[contMin] = array[i];
      contMin++;
    }else{
      superiori[contSup] = array[i];
      contSup++;
    }
  }
  /*riuniamo*/
  memcpy(array, minori, nInfSoglia*sizeof(int));
  memcpy(&array[nInfSoglia], superiori, (size-nInfSoglia)*sizeof(int));

  /*debug*/
  printf("[p]Preordine: ");
  printIntArray(array, size);
}
