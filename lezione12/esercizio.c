#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#define HEAP_LIMIT 100

//struttura dati heap
typedef struct {
  int bytesLeft;             // bytes attualmente disponibili
  pthread_cond_t cond;    // condition variable
  pthread_mutex_t mutex;  // mutex associato alla condition variable
} heap;

//variabili globali
heap h; // un nuovo heap

//struttura dati argomento per i thread
typedef struct{
    int mem, sec;
} threadArg;

//prototipi
int heap_init(heap *h, int maxSize);
int allocate_memory(heap *h, int size);
int free_mem(heap *h, int size);
void *tbody(void *arg);

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
    if(size > HEAP_LIMIT){
        return -1;
    }
    e = pthread_mutex_lock(&h->mutex);
    if(e!=0) return e;
    while(h->bytesLeft < size){ //mentre non ho abbastanza spazio libero ...
        e = pthread_cond_wait(&h->cond, &h->mutex); //attendiamo si liberi spazio
        if(e!=0) return e;
    }
    h->bytesLeft -= size;
    printf("sto allocando %d bytes di memoria, ne rimangono ancora %d disponibili\n", size, h->bytesLeft);
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



void handler(int s)
{
    int e;
    pthread_t nuovoThread;
    /*
    * C'e' infine un punto molto importante riguardante la creazione
    * dei thread da parte del signal handler. Se usiamo la solita tecnica
    * di passare attraverso la pthread_create il puntatore ad una struttura
    * contenente l'input per il thread, è importante che questo puntatore
    * non si riferisca ad una variabile locale dell'handler in quanto 
    * l'handler può terminare prima che il thread abbia letto l'input: in
    * questi caso il thread tenterebbe di accedere ad una variabile che
    * non esiste più con possibile segmentation fault o risultati errati.
    * Quello che è necessario fare e' passare il puntatore ad una struttura
    * allocata con malloc che sopravvive alla terminazione del signal handler.
    * In questo caso il thread dopo aver letto i dati deve assicurarsi di effettuare
    * il free della struttura.
    */
    threadArg *a = (threadArg*) malloc(sizeof(threadArg));
    printf("Segnale %d ricevuto dal processo %d\n", s, getpid());

    if(s==SIGUSR1) {
        //  ogni volta che viene inviato il segnale USR1 il programma deve 
        //  chiedere (usando printf/scanf) due interi mem e sec e lancia un
        //  nuovo thread che alloca mem byte di memoria dall'heap condiviso,
        //  attende per sec secondi con una sleep, dealloca i mem byte e termina.
        printf("Quanti byte allocare ? ");
        e = scanf("%d", &a->mem);
        assert(e>=0);
        printf("\nQuanti secondi di durata ? ");
        e = scanf("%d", &a->sec);
        assert(e>=0);
        //lancia il thread
        e = pthread_create(&nuovoThread, NULL, tbody, a);
        assert(e==0);
        return;
    }else if(s==SIGUSR2){
        //  ogni volta che viene inviato il segnale USR2 il programma deve 
        //  visualizzare la quantità di memoria dell'heap attualmente
        //  disponibile.

        //h è condiviso, quindi regione critica
        pthread_mutex_lock(&h.mutex);
        printf("Memoria disponibile nell'heap: %d\n", h.bytesLeft);
        pthread_mutex_unlock(&h.mutex);
    } 
}

//  ogni volta che viene inviato il segnale USR1 il programma deve 
//  chiedere (usando printf/scanf) due interi mem e sec e lancia un
//  nuovo thread che alloca mem byte di memoria dall'heap condiviso,
//  attende per sec secondi con una sleep, dealloca i mem byte e termina.
void *tbody(void *arg){
    int e;
    threadArg *a = (threadArg *)arg;
    e = allocate_memory(&h, a->mem); //alloca mem byte
    if(e == -1){
        //tentativo di allocare piu memoria di quella disponibile
        printf("Errore, tentativo di allocare %d bytes, ma solo %d disponibili\n", a->mem, h.bytesLeft);
        pthread_exit(NULL);
    }
    printf("[thread]Ho allocato %d nella heap\n", a->mem);
    assert(e==0);
    printf("[thread]Vado a dormire per %d sec\n", a->sec);
    sleep(a->sec); //esegui sec secondi
    printf("[thread]Sono passati %d sec, disalloco %d bytes e termino\n", a->sec, a->mem);
    e= free_mem(&h, a->mem);
    assert(e==0); 

    //dobbiamo eseguire la free sull'argomento, allocato con malloc dal chiamante
    free(a);

    pthread_exit(NULL); 
}

int main(int argc, char *argv[])
{
    heap_init(&h, HEAP_LIMIT); //inizializazione

    // definisce signal handler 
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);     // setta a "insieme vuoto" sa.sa_mask maschera di segnali da bloccare 
    sa.sa_flags = SA_RESTART;     // restart system calls  if interrupted
    sigaction(SIGUSR1,&sa,NULL);  // handler per USR1
    sigaction(SIGUSR2,&sa,NULL);  // handler per USR2

    //h è regione critica
    pthread_mutex_lock(&h.mutex);
    printf("Il processo con pid %d ha un heap di %d bytes\n", getpid(), h.bytesLeft);
    pthread_mutex_unlock(&h.mutex);

    //attendiamo un segnale
    while(true){
        sleep(5);//la sleep si interrompe se arriva un segnale
    }

}