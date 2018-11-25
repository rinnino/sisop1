/*
* testo esame 24/1/17, esercizio thread
*/
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>      
#include <sys/types.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define DIM_BUFF 10

// struct contenente i parametri di input e output di ogni thread 
typedef struct {
    int label;    //etichetta per il thread
    int *indiceBuff;   //indice circolare buffer
    int *buffer;  //buffer numeri
    FILE *f; //file da cui leggere

    //mutex e semafori per produttore consumatore
    pthread_mutex_t *mutex_buffer, *mutex_file;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;  
} dati;

//funzione eseguita dal thread
//legge dal file un numero e lo mette nel buffer
//è un thread produttore
void *tbody(void *arg){
    int e,n=0;
    dati *a = (dati *)arg;
    while(true){
        /*regione critica*/
            //lettura da file
            pthread_mutex_lock(a->mutex_file); //lock mutex file
            e = fscanf(a->f, "%d", &n);
            pthread_mutex_unlock(a->mutex_file); //unlock mutex file
        /*fine regione critica*/

        //stampa debug
        printf("[thread %d] leggo %d\n", a->label, n);

        //desidero produrre, devo fare la down sul semaforo degli
        //elementi vuoti
        sem_wait(a->sem_free_slots);

        /*regione critica*/
            pthread_mutex_lock(a->mutex_buffer); //lock mutex buffer
            if(e==EOF){
                a->buffer[*(a->indiceBuff)] = -1;//scrittura nel buffer
                //debug
                printf("[thread %d] ho scritto -1 nella posizione %d\n",a->label, *a->indiceBuff);
            }else{
                a->buffer[*(a->indiceBuff)] = n*n;//scrittura nel buffer
                //debug
            printf("[thread %d] ho scritto %d che e il quadrato di %d nella posizione %d\n",a->label, n*n, n, *a->indiceBuff);
            }
            //avanzo il buffer circolare
            *(a->indiceBuff) =  ((*(a->indiceBuff)) + 1)%DIM_BUFF;
            pthread_mutex_unlock(a->mutex_buffer); //unlock mutex file
        /*fine regione critica*/

        //ho prodotto, devo fare una up sul semaforo degli elementi pieni
        sem_post(a->sem_data_items);

        if(e==EOF){
            printf("[thread %d] terminato.\n", a->label);
            break;
        }
    }
    pthread_exit(NULL); 
}

int main(int argc, char *argv[])
{
    // leggi input
    if(argc!=3) {
        printf("Uso\n\t%s file numthread\n", argv[0]);
    exit(1);
    }
    //variabili
    int e,indexBuff=0;

    //buffer e relativo mutex
    int buffer[DIM_BUFF];
    memset(buffer, 0, DIM_BUFF*sizeof(int)); //pulizia buffer
    pthread_mutex_t mutexBuff = PTHREAD_MUTEX_INITIALIZER;


    //numero threads
    int p = atoi(argv[2]);

    //dati per i thread
    pthread_t t[p];
    dati a[p];

    //mutex i/o file
    pthread_mutex_t mutexFile = PTHREAD_MUTEX_INITIALIZER;
    
    //apertura file
    FILE *file = fopen(argv[1], "r");

    //semaforo slot pieni e sua init
    sem_t slot_pieni;
    e = sem_init(&slot_pieni, 0, 0);
    assert(e==0);

    //semaforo slot pieni e sua init
    sem_t slot_vuoti;
    e = sem_init(&slot_vuoti, 0, DIM_BUFF);
    assert(e==0);

    //creazione threads
    for(int i=0; i<p; i++){
        a[i].f = file;
        a[i].buffer = buffer;
        a[i].indiceBuff = &indexBuff;
        a[i].label = i;
        a[i].mutex_file = &mutexFile;
        a[i].mutex_buffer = &mutexBuff;
        a[i].sem_data_items = &slot_pieni;
        a[i].sem_free_slots = &slot_vuoti;
        e = pthread_create(&t[i], NULL, tbody, (void *) &a[i]);
        assert(e==0);
    }

    //ora possiamo diventare consumatori del buffer

    //debug
    printf("[padre] buffer:");
    for(int i=0; i<DIM_BUFF; i++){
        printf( "[%d]", buffer[i]);
    }
    printf("\n");

    //variabile per somma
    int sum=0;

    //consumiamo
    for(int indexLetturaBuff=0, sumThreads=p; sumThreads>0;){ //finche ce almeno un produttore .... (sumThreads>0)
        //desidero consumare, devo fare la down sul semaforo degli
        //elementi pieni
        sem_wait(&slot_pieni);

        /*regione critica*/
            pthread_mutex_lock(&mutexBuff); //lock mutex buffer
            if(buffer[indexLetturaBuff]==-1){ //elemento -1, un produttore ha terminato
                printf("[padre consumatore] ho letto %d nella posizione %d\n", buffer[indexLetturaBuff], indexLetturaBuff);
                sumThreads--; // una posizione -1 significa che un produttore ha finito
            }
            else{ //elemento diverso -1, sommiamolo
                sum += buffer[indexLetturaBuff];
                printf("[padre consumatore] ho letto %d nella posizione %d\n", buffer[indexLetturaBuff], indexLetturaBuff);
            }
            //avanzo il buffer circolare
            indexLetturaBuff =  (indexLetturaBuff + 1)%DIM_BUFF;
            pthread_mutex_unlock(&mutexBuff); //unlock mutex file
        /*fine regione critica*/

        //ho consumato, devo fare una up sul semaforo degli elementi vuoti
        sem_post(&slot_vuoti);
    }

    //attesa threads
    // join dei thread e calcolo risultato
    for(int i=0;i<p;i++) {
        e = pthread_join(t[i], NULL);
        if(e==0) {
            fprintf(stderr,"Thread %d terminato\n",i);
        }else{
            fprintf(stderr,"Errore join %d\n",e);
        }
    }
    
    printf("[padre] la somma è %d\n", sum);

}