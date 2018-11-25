#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// ===== Esempi di uso delle condition variables ====

// Nota: le funzioni che utilizzano le condition variables
//   pthread_cond_init
//   pthread_cond_wait
//   pthread_cond_signal
//   pthread_cond_broadcast
// tutte restituiscono un intero che e' 0 se non ci sono stati errori,
// altrimenti restituiscono il codice dell'errore (come le funzioni
// che utilizzano i mutex).  



// ========  Esempio 1 

// Uso di una Condition Variable per gestione di un heap di memoria 

// !!!! Orribili variabili globali. 
// !!!! Nei programmi veri li mettiamo tutti insieme in una struct

// numero iniziale di bytes disponibili
#define MAX_HEAP_SIZE 100000
int bytesLeft = MAX_HEAP_SIZE;
// lock and condition variable usate insieme  
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;;

// prototipi funzioni per allocazione e deallocazione memoria
// il codice non fornito
void *get_heapmemory(int);    
void free_heapmemory(void *);


// la cv c e' protetta dal mutex m e si riferisce alla variabile bytesLeft
//   chiunque modifica byteLeft deve farlo sotto il lock di m
//   per mettersi in attesa del segnale c o per mandare il segnale c
//   si deve essere sotto il lock m 

// funzione chiamata da ogni thread che ha bisogno di memoria
// se non c'e' abbastanza memoria attende che se ne liberi
void * allocate(int size) {
  pthread_mutex_lock(&m);
  while (bytesLeft < size)
    // aspetto che qualcuno attraverso c indichi che ci sono novita' su byteLeft
    pthread_cond_wait(&c, &m);  // il mutex viene rilascato dalla cond_wait
  // all'uscita della cond_wait il mutex e' ottenuto   
  void *ptr = get_heapmemory(size);  // ottiene il puntatore alla zona di memoria
  bytesLeft -= size;                 // aggiorna byteLeft
  pthread_mutex_unlock(&m);
  return ptr;
}

// funzione chiamata da ogni thread che restituisce della memoria
void free_mem(void *ptr, int size) {
  pthread_mutex_lock(&m);
  free_heapmemory(ptr);        // restituisce la memoria usata
  bytesLeft += size;            // modifico byteLeft sotto mutex
  // tutti quelli che stanno aspettando su c sono avvertiti ...
  pthread_cond_broadcast(&c);   // ... ma non partono subito perche' c'e' il mutex
  pthread_mutex_unlock(&m);     // rilascio mutex
}




// ===== Esempio 2 ==================

// Condition variable usata per realizzare un reader/writer lock

// !!!! Orribili variabili globali

// numero di reader e scrittura in corso  
int readers=0; 
bool writing=false;

// condition variable e mutex associato
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;;

// in questo esempio m e c si riferiscono alle due variabili 
// readers e writing. Chi le cambia deve eseguire il lock su m e se 
// sono state liberate delle risorse deve mandare il segnale attarverso c 
// I thread che devono eseguire un lock ma non possono farlo devono mettersi in wait su c
void read_lock()
{
  pthread_mutex_lock(&m);
  while(writing) 
    pthread_cond_wait(&c, &m);   // attende fine scrittura
  readers++;
  pthread_mutex_unlock(&m);
}

void read_unlock()
{
  pthread_mutex_lock(&m);
  readers--;                  // cambio di stato       
  if(readers==0) pthread_cond_signal(&c); // da segnalare ad un solo writer
  pthread_mutex_unlock(&m);
}
  
void write_lock()
{
  pthread_mutex_lock(&m);
  while(writing || readers>0) 
    pthread_cond_wait(&c, &m);   // attende fine scrittura o lettura
  writing = true;
  pthread_mutex_unlock(&m);
}

void write_unlock()
{
  pthread_mutex_lock(&m);
  writing=false;               // cambio stato
  pthread_cond_broadcast(&c);  // da segnalare a tutti quelli in attesa 
  pthread_mutex_unlock(&m);
}



// ======= Esempio 3

// Realizzazione di un semaforo usando mutex+condition variable

// il semaforo blocca solo sullo 0
// la condition var e' solo un segnale quindi il modo di bloccare 
// dipende dal test prima del cond_ wait quindi puo' essere piu' generale

// un semaforo si puo' realizzare con mutex + condition var
// come mostrato qui sotto


// realizzazione di un semaforo mediante Mutex + CV

typedef struct {
  int tot;  // valore del semaforo, non deve mai diventare negativo
  pthread_cond_t cond;   // condition variable
  pthread_mutex_t mutex; // mutex associatcondition variable
} zem;

// inzializza semaforo al valore q
void zem_init(zem *z, int q)
{
  z->tot = q;
  pthread_cond_init(&z->cond,NULL);
  pthread_mutex_init(&z->mutex,NULL);
}

// analoga alla sem_wait
void zem_down(zem *z, int q)
{
  pthread_mutex_lock(&z->mutex);
  while(z->tot-q<0)
    pthread_cond_wait(&z->cond,&z->mutex);
  z->tot -= q;
  pthread_mutex_unlock(&z->mutex);
}


// analoga alla sem_post
void zem_up(zem *z, int q)
{
  pthread_mutex_lock(&z->mutex);
  z->tot+=q;
  pthread_cond_broadcast(&z->cond);
  pthread_mutex_unlock(&z->mutex);
}


