#include "xerrori.h"
#include <time.h>

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

// genera array di n elementi con interi random tra 0 e 1999
int *random_array(int n)
{
  assert(n>0);
  srand(time(NULL));
  int *a = malloc(n* sizeof(int));
  assert(a!=NULL);
  for(int i=0;i < n;i++)
    a[i] = (int) random() % 2000;
  return a;
}

//conta quanti pari o dispari ci siano in un vettore di interi
// con dimensione size. se dispari == 0, conta i pari, i dispari altrimenti
int contaParidis(int *array, int dispari, int size){
  if(dispari < 0 || dispari > 1){die("Problema contaParidis()");}
  int ris=0;
  for(int i = 0; i < size; i++){
    if((array[i]%2) == dispari){ris++;}
  }
  return ris;
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

//preleva i dispari od i pari da array e li inserisce in arrayPariDis
// senza ordinare. Preleva i pari se dispari = 0; i dispari se dispari = 1.
//size è la dimensione di array.
//la dimensione di arrayPariDis deve essere prevista prima dell'uso di questa
//funzione. Restituisce il numero di numeri scritti in arrayPariDis
int popolaPariDis(int *array, int *arrayPariDis, int dispari, int size){
  if(dispari < 0 || dispari > 1){die("Problema contaParidis()");}
  int ris=0;
  for(int i = 0; i < size; i++){
    if((array[i]%2) == dispari){
      arrayPariDis[ris] = array[i];
      ris++;
    }
  }
  return ris;
}

int popolaInfSoglia(int *array, int *arrayRis, int soglia, int size){
  if(array == NULL || arrayRis == NULL){die("Problema NULL popolaMinSoglia");}
  int ris=0;
  for(int i = 0; i < size; i++){
    if(array[i] < soglia){
      arrayRis[ris] = array[i];
      ris++;
    }
  }
  return ris;
}

int popolaOverSoglia(int *array, int *arrayRis, int soglia, int size){
  if(array == NULL || arrayRis == NULL){die("Problema NULL popolaOverSoglia");}
  int ris=0;
  for(int i = 0; i < size; i++){
    if(array[i] >= soglia){
      arrayRis[ris] = array[i];
      ris++;
    }
  }
  return ris;
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

int main(int argc, char const *argv[]) {

  /* variabili */
  int e;

  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s n\n\n", argv[0]);
    exit(1);
  }

  /* creiamo una pipe*/
  int p[2];
  e = pipe(p);
  if(e>0) {die("Problema creazione pipe\n");}

  /* creiamo un array grande n e popoliamolo con 2000 numeri
  casuali tra 0 e 1999*/
  int *array;
  array = random_array(atoi(argv[1]));

  /* debug: stampo vettore*/
  printIntArray(array, atoi(argv[1]));

  /* debug: stampiamone la somma */
  printf("La somma dei numeri e: %ld\n", sumIntArray(array, atoi(argv[1])));

  /* contiamo quanti min 1000 ci sono*/
  int nInfSoglia = contaInfSoglia(array, 1000, atoi(argv[1]));

  /*creo array min 1000*/
  int arrayInfSoglia[nInfSoglia];

  /* contiamo quanti sup uguale 1000 ci sono*/
  int nSupSoglia = contaSupSoglia(array, 1000, atoi(argv[1]));

  /*creo array sup uguali 1000*/
  int arraySupSOglia[nSupSoglia];

  /*fork, creazione figlio*/
  pid_t pid = xfork(__FILE__, __LINE__);
  if(pid == 0){
    /* codice figlio*/

    /*pipe solo scrittura*/
    close(p[0]);


    /*popoliamo l'array dei superiori uguali 1000*/
    popolaOverSoglia(array, arraySupSOglia, 1000, atoi(argv[1]));


    /* debug: stampo array sup uguali soglia*/
    printf("[f]Superiori o uguali a 1000 disordinati: ");
    printIntArray(arraySupSOglia, nSupSoglia);

    /* ordino superiori uguali 1000*/
    qsort(arraySupSOglia, nSupSoglia, sizeof(int), intcmp);

    /* debug: stampo superiori uguali 1000 a video*/
    printf("[f]Superiori o uguali a 1000 ordinati: ");
    printIntArray(arraySupSOglia, nSupSoglia);

    /*invio tramite pipe arrayDispari al padre*/
    e = write(p[1], arraySupSOglia, nSupSoglia*sizeof(int));
    if(e != nSupSoglia*sizeof(int)){
      perror("[f]Problema con la write pipeDispari");
      exit(1);
    }

    /*chiusura pipe*/
    close(p[1]);

    /*rilascio strutture dati figlio*/
    free(array);

    /*termine figlio*/
    exit(0);
  }else{
    /*codice padre*/
    /*pipe solo lettura*/
    close(p[1]);

    /*popoliamo l'array dei minori di 1000*/
    popolaInfSoglia(array, arrayInfSoglia, 1000, atoi(argv[1]));

    /* debug: stampo array inf 1000 a video*/
    printf("[p]Minori di 1000 disordinati: ");
    printIntArray(arrayInfSoglia, nInfSoglia);

    /* ordino inf 1000*/
    qsort(arrayInfSoglia, nInfSoglia, sizeof(int), intcmp);

    /* debug: stampo array inf 1000 ordinati a video*/
    printf("[p]Minori di 1000 ordinati: ");
    printIntArray(arrayInfSoglia, nInfSoglia);

    /* lettura dalla pipe*/
    e = read(p[0], arraySupSOglia, nSupSoglia*sizeof(int));
    if(e != nSupSoglia*sizeof(int)){
      perror("[p]Problema con la read() pipe p");
      exit(1);
    }

    /* debug : stampo superiori a 1000 dalla pipe a video*/
    printf("[p]Superiori o uguali a 1000 ordinati letti dalla pipe: ");
    printIntArray(arraySupSOglia, nSupSoglia);

    /* riverso i 2 array ordinati nell array di partenza */
    memcpy(array, arrayInfSoglia, nInfSoglia*sizeof(int));
    memcpy(&array[nInfSoglia], arraySupSOglia, nSupSoglia*sizeof(int));

    /* stampo array originale ordinato*/
    printIntArray(array, atoi(argv[1]));

    /*debug: verifica ordinamento finale*/
    if(check_sort(array, atoi(argv[1]))){
      printf("Array ordinato correttamente.\n");
    }else{
      die("Problema Ordinamento array\n");
    }

    /* debug: stampiamo la somma de vettore ordinato */
    printf("La somma dei numeri e: %ld\n", sumIntArray(array, atoi(argv[1])));

    /*chiudiamo la pipe in lettura*/
    close(p[0]);

    /*rilascio strutture dati heap*/
    free(array);

  }

  return 0;
}
