#include "xerrori.h"
#include <time.h>
#include <math.h>

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
  int flagFiglio; //indica il numero del figlio
  pid_t pid;
  int p0[2], p1[2]; // pipe per i rispettivi figli
  int status;


  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s n\n\n", argv[0]);
    exit(1);
  }

  /* creiamo una pipe*/
  e = pipe(p0);
  if(e>0) {die("Problema creazione pipe\n");}
  e = pipe(p1);
  if(e>0) {die("Problema creazione pipe\n");}

  /* creiamo un array grande n e popoliamolo con 2000 numeri
  casuali tra 0 e 1999*/
  int *array;
  array = random_array(atoi(argv[1]));

  /* debug: stampo vettore*/
  printIntArray(array, atoi(argv[1]));

  /* debug: stampiamone la somma */
  printf("La somma dei numeri e: %ld\n", sumIntArray(array, atoi(argv[1])));

  /* creazione figli */
  for(flagFiglio = 0; flagFiglio < 2; flagFiglio++){
    printf("[p]Creazione figlio %d\n", flagFiglio);
    pid = xfork(__FILE__, __LINE__);
    if(pid==0){break;} // un figlio non deve creare altri figli ...
  }
  if(pid==0){
    /*codice figli*/
    if(flagFiglio==0){
      /*codice primo figlio*/

      /*debug*/
      printf("[f%d]Ciao\n", flagFiglio);

      /*chiudiamo p1 e teniamo p0 in sola scrittura*/
      close(p0[0]);
      close(p1[0]);
      close(p1[1]);

      /*creiamo un nuovo array per ordinare da 0 a n/2*/
      int arrayRis[atoi(argv[1])/2];

      /*lo popoliamo coi valori da 0 a n/2*/
      memcpy(arrayRis, array,
         sizeof(int)*(atoi(argv[1]))/2);

      /* ordiniamo il nuovo array ottenuto */
      qsort(arrayRis, atoi(argv[1])/2, sizeof(int), intcmp);

      /* debug: stampo array ris di p0 */
      printf("[f%d]Array da zero a n/2 ordinato: ", flagFiglio);
      printIntArray(arrayRis, atoi(argv[1])/2);

      /*invio a pipe p0 numero per numero*/
      for(int i=0; i<(atoi(argv[1])/2); i++){
        e = write(p0[1], &arrayRis[i], sizeof(int));
        printf("[f%d]Invio %d\n", flagFiglio, arrayRis[i]);
        if(e!=sizeof(int)){
          perror("[f0]Problema con la write su p0[1]");
          exit(1);
        }
      }

      /*debug*/
      printf("[f%d]fine invio su pipe.\n", flagFiglio);

      /* chiusura p0*/
      close(p0[1]);

      /*deallocazione heap*/
      free(array);
    }
    if(flagFiglio==1){
      /*codice secondo figlio*/

      /*debug*/
      printf("[f%d]Ciao\n", flagFiglio);

      /*chiudiamo p0 e teniamo p1 in sola scrittura*/
      close(p1[0]);
      close(p0[0]);
      close(p0[1]);

      /*se n è dispari questo processo deve tenere un numero in piu*/
      int dim;
      if(atoi(argv[1])%2 == 0){
        /*pari*/
        dim = atoi(argv[1])/2;
      }else{
        dim = (atoi(argv[1])/2) + 1;
      }

      /*creiamo un nuovo array per ordinare da n/2 a n*/
      int arrayRis[dim];

      /*lo popoliamo coi valori da n/2 a n*/
      memcpy(arrayRis, &array[atoi(argv[1])/2], sizeof(int)*dim);

      /* ordiniamo il nuovo array ottenuto */
      qsort(arrayRis, dim, sizeof(int), intcmp);

      /* debug: stampo array ris di p0 */
      printf("[f%d]Array da n/2 ad n ordinato: ", flagFiglio);
      printIntArray(arrayRis, dim);

      /*invio a pipe p1 numero per numero*/
      for(int i=0; i<dim; i++){
        e = write(p1[1], &arrayRis[i], sizeof(int));
        printf("[f%d]Invio %d\n", flagFiglio, arrayRis[i]);
        if(e!=sizeof(int)){
          perror("[f1]Problema con la write su p1[1]");
          exit(1);
        }
      }

      /*debug*/
      printf("[f%d]fine invio su pipe.\n", flagFiglio);

      /* chiusura p1*/
      close(p1[1]);

      /*deallocazione heap*/
      free(array);
    }
  }else{
    /*
     * codice padre
     */

    /*Chiusura lato scrittura delle pipe*/
    close(p0[1]);
    close(p1[1]);

    int n0,n1; /*numeri letti da p0 e p1*/
    bool isP0Empty = false;
    bool isP1Empty = false;
    int i = 0;

    /*prima lettura da entrambe le pipe*/
    e = read(p0[0], &n0, sizeof(int));
    if(e < 0){
      perror("problema prima lettura p0");
      exit(1);
    }
    if(e == 0){isP0Empty = true;}

    e = read(p1[0], &n1, sizeof(int));
    if(e < 0){
      perror("problema prima lettura p1");
      exit(1);
    }
    if(e == 0){isP1Empty = true;}

    /*debug*/
    printf("[p]Leggo %d da p0 e %d da p1\n", n0, n1);

    while(isP0Empty == false && isP1Empty == false){
      if(n0<n1){
        array[i] = n0;
        e = read(p0[0], &n0, sizeof(int));
        if(e < 0){
          perror("problema lettura p0");
          exit(1);
        }
        i++;
        if(e != sizeof(int)){isP0Empty = true; printf("[p]p0 vuota.\n");}
        if(e == sizeof(int)){ printf("[p]Leggo %d da p0\n", n0);}
      }else{
        array[i] = n1;
        e = read(p1[0], &n1, sizeof(int));
        if(e < 0){
          perror("problema lettura p1");
          exit(1);
        }
        i++;
        if(e != sizeof(int)){isP1Empty = true; printf("[p]p1 vuota.\n");}
        if(e == sizeof(int)){ printf("[p]Leggo %d da p1\n", n1);}
      }
    }

    if(isP0Empty == true){
      /*leggo ultimi valori da p1*/
      array[i] = n1;
      i++;
      while(isP1Empty == false){
        e = read(p1[0], &n1, sizeof(int));
        if(e < 0){
          perror("problema lettura p1");
          exit(1);
        }
        if(e == sizeof(int)){ printf("[p]Leggo %d da p1\n", n1); array[i] = n1; i++;}
        if(e != sizeof(int)){isP1Empty = true; printf("[p]p1 vuota.\n");}
      }
    }else{
      /*leggo ultimi valori da p0*/
      array[i] = n0;
      i++;
      while(isP0Empty == false){
        e = read(p0[0], &n0, sizeof(int));
        if(e < 0){
          perror("problema lettura p0");
          exit(1);
        }
        if(e == sizeof(int)){ printf("[p]Leggo %d da p0\n", n0); array[i] = n0; i++;}
        if(e != sizeof(int)){isP0Empty = true; printf("[p]p0 vuota.\n");}
      }
    }


    /*debug: stampa finale*/
    printf("[p]Vettore ordinato ricavato dalle pipe: ");
    printIntArray(array, atoi(argv[1]));

    /*chiusura delle pipe di lettura*/
    close(p0[0]);
    close(p1[0]);

    /*deallocazione heap*/
    free(array);

    /* attesa figli */
    xwait(&status, __FILE__, __LINE__);
  }
}
