#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#define HEAP_LIMIT 1000000

//variabili globali
heap h; // un nuovo heap

//prototipi
int heap_init(heap *h, int maxSize);
int allocate_memory(heap *h, int size);
int free_mem(heap *h, int size);

//struttura dati heap
typedef struct {
  int bytesLeft;             // bytes attualmente disponibili
  pthread_cond_t cond;    // condition variable
  pthread_mutex_t mutex;  // mutex associato alla condition variable
} heap;

//heap_init inizializza la struct heap e deve essere chiamata
//una volta sola. maxSize è la quantità di meoria assegnata all'heap.
int heap_init(heap *h, int maxSize){
    int e;
    h->bytesLeft = maxSize;
    e = pthread_cond_init(&h->cond, NULL);
    if(e!=0) return e;
    e = pthread_mutex_init(&h->mutex, NULL);
    if(e!=0) return e;
    return 0;
}

//richiede size bytes di memoria
//non davvero ma solo simula, così tanto per fare esercizio
int allocate_memory(heap *h, int size){
    int e;
    e = pthread_mutex_lock(&h->mutex);
    if(e!=0) return e;
    while(h->bytesLeft < size){
        e = pthread_cond_wait(&h->cond, &h->mutex);
        if(e!=0) return e;
    }
    h->bytesLeft -= size;
    printf("sto allocando %d bytes ti memoria, ne rimangono ancora %d disponibili", size, h->bytesLeft);
    e = pthread_mutex_unlock(&h->mutex);
    if(e!=0) return e;
    return 0;
}

//restituisce al sistema size bytes di memoria 
//Per le ultime 2 si veda l'esempio fatto a lezione:
//non mettete le chiamate alle funzioni get_heapmemory
//e free_heapmemory che erano solo dimostrative.
int free_mem(heap *h, int size){
    int e;
    e = pthread_mutex_lock(&h->mutex);
    if(e!=0) return e;
    h->bytesLeft += size;
    e = pthread_cond_broadcast(&h->cond);
    if(e!=0) return e;
    e = pthread_mutex_unlock(&h->mutex);
    if(e!=0) return e;
    return 0;
}

//Per testare il funzionamento del vostro codice aggiungere un main() 
//che inizializza l'heap e che definisce due handler per i segnali USR1
// e USR2 in modo che:

//  ogni volta che viene inviato il segnale USR1 il programma deve 
//  chiedere (usando printf/scanf) due interi mem e sec e lancia un
//  nuovo thread che alloca mem byte di memoria dall'heap condiviso,
//  attende per sec secondi con una sleep, dealloca i mem byte e termina.

//  ogni volta che viene inviato il segnale USR2 il programma deve 
//  visualizzare la quantità di memoria dell'heap attualmente
//  disponibile.

//Dopo aver definito gli handler il programma principale deve stampare
//il numero del processo ed entrare in un ciclo infinito
//(ma non in busy-waiting) in attesa che vengano inviati dalla linea di
//comando (da un altro terminale) i segnali USR1 e USR2.

//Facendo visualizzare ai thread dei messaggi che indicano l'inizio e 
//la fine della propria attività verificare che il comportamento sia
// quello atteso, nel senso che se un thread cerca di allocare più
// memoria di quanta ne sia disponibile esso si interrompe fino a
// quando gli altri thread non liberano sufficiente memoria.


//  ogni volta che viene inviato il segnale USR1 il programma deve 
//  chiedere (usando printf/scanf) due interi mem e sec e lancia un
//  nuovo thread che alloca mem byte di memoria dall'heap condiviso,
//  attende per sec secondi con una sleep, dealloca i mem byte e termina.
void handler(int s)
{
    int mem, sec, e;
    printf("Segnale %d ricevuto dal processo %d\n", s, getpid());
    if(s==SIGUSR1) {
    // dopo aver fatto girare il programma la prima volta
    // s-commentate l'istruzione kill e osservate cosa succede
    //kill(getpid(),SIGUSR1);
    printf("Quanti byte allocare ? ");
    scanf(&mem);
    printf("\nQuanti secondi di durata ? ");
    scanf(&sec);

    e = allocate_memory(&h, mem); //alloca mem byte
    if(e!=0) return e;
    sleep(sec); //esegui sec secondi
    e= free_mem(&h, mem);
    if(e!=0) return e;
  } 
}

int main(int argc, char *argv[])
{
    heap_init(&h, HEAP_LIMIT); //inizializazione
}