#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "xerrors.h"

//prototipi
int *random_array(int n);
int intcmp(const void *a, const void *b);
void array_sort(int *a, int n);

int main(int argc, char *argv[])
{
    int n; //numero elemento array
    int *array;
    pid_t pid;

    // crea pipe per la comnicazione padre figlio.
    int pipe[2];
    xpipe(pipe,__LINE__,__FILE__);

    //inizializzazione rand
    srand(time(NULL));

    //input previsto: nomeprogramma n
    //dove n è il numero di elementi dell'array
    // leggi input
    if(argc!=2) {
        printf("Uso\n\t%s numero_elementi_array\n", argv[0]);
        exit(1);
    }
    //otteniamo n dall'argv
    n = atoi(argv[1]);
    //popolo l'array
    array = random_array(n);

    //debug stampa array
    printf("[padre] Array di partenza: ");
    for(int i=0; i<n; i++){
        printf(" [%d]", array[i]);
    }
    printf("\n");

    //fork
    pid = xfork(__LINE__,__FILE__);
    if(pid==0){
        /*
         * Codice processo figlio
         */

        //pipe: ci serve solamente il canale di scrittura
        close(pipe[0]); //elimino canale lettura

        //contiamo quanti elementi maggiori uguali RAND_MAX/2
        int numElFiglio = 0;
        for(int i=0; i<n; i++){
            if(array[i] >= (RAND_MAX/2)){
                numElFiglio++;
            }
        }

        //debug stampa numero elementi figlio
        printf("[figlio] numeri maggiori uguali di %d : %d\n", RAND_MAX, numElFiglio);

        //creiamo un array per contenerli
        int arrayFiglio[numElFiglio];

        //popoliamolo
        for(int i=0, cont=0; i<n; i++){
            //cont serve per tenere conto della posizione nell
            //arrayFiglio
            if(array[i] >= (RAND_MAX/2)){
                arrayFiglio[cont] = array[i];
                cont++;
            }
        }

        //stampa di prova Figlio
        printf("[figlio] numeri di competenza:");
        for(int i=0; i<numElFiglio; i++){
            printf(" [%d]", arrayFiglio[i]);
        }
        printf("\n");

        //ordinamento array figlio
        array_sort(arrayFiglio, numElFiglio);

        //stampa di prova arrayFiglio
        printf("[figlio] array ordinato : ");
        for(int i=0; i<numElFiglio; i++){
            printf(" [%d]", arrayFiglio[i]);
        }
        printf("\n");

        //invio al padre di arrayFiglio con pipe
        printf("[figlio] invio dell'array al padre con la pipe ...\n");
        int e  = write(pipe[1], arrayFiglio, numElFiglio * sizeof(int));
        assert(e == numElFiglio * sizeof(int));
        close(pipe[1]);

        //fine processo figlio
        exit(0);  


    }else{
        /*
         * Codice processo padre
         */

        //pipe: ci serve solamente il canale di lettura
        close(pipe[1]); //elimino canale scrittura

        //contiamo quanti elementi minori RAND_MAX/2
        int numElPadre = 0;
        for(int i=0; i<n; i++){
            if(array[i] < (RAND_MAX/2)){
                numElPadre++;
            }
        }

        //debug stampa numero elementi padre
        printf("[padre] numeri minori di %d : %d\n", RAND_MAX, numElPadre);

        //creiamo un array per contenerli
        int arrayPadre[numElPadre];

        //popoliamolo
        for(int i=0, cont=0; i<n; i++){
            //cont serve per tenere conto della posizione nell
            //arrayPadre
            if(array[i] < (RAND_MAX/2)){
                arrayPadre[cont] = array[i];
                cont++;
            }
        }

        //stampa di prova arrayPadre
        printf("[padre] numeri di competenza:");
        for(int i=0; i<numElPadre; i++){
            printf(" [%d]", arrayPadre[i]);
        }
        printf("\n");

        //ordinamento array padre
        array_sort(arrayPadre, numElPadre);

        //stampa di prova arrayPadre
        printf("[padre] array ordinato : ");
        for(int i=0; i<numElPadre; i++){
            printf(" [%d]", arrayPadre[i]);
        }
        printf("\n");

        //utilizziamo l'array iniziale per popolare quello ordinato
        //popoliamo l'area di competenza del padre
        for(int i=0; i<numElPadre; i++){
            array[i] = arrayPadre[i];
        }

        //ora popoliamo dalla pipe coi valori del figlio
        for(int e, intFromPipe, cont=numElPadre;;){
            e = read(pipe[0], &intFromPipe, sizeof(int));
            if(e==0) break; //fine pipe
            if(e!=sizeof(int)) {perror("Errore lettura pipe"); exit(1);}

            //stampa debug
            printf("[padre] ricezione int da pipe: %d\n", intFromPipe);

            //inseriamo l'intero nell'array ordinato
            array[cont] = intFromPipe;
            //avanziamo di posizione
            cont++;
        }

        //a questo punto l'array e completo, stampiamolo:
        printf("[padre] array completo ordinato:");
        for(int i=0; i<n; i++){
            printf(" [%d]", array[i]);
        }
        printf("\n");

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