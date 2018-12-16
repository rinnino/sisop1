#include "xerrors.h"
#define NAME "/memcond"
#define NAME_SEM "/semcond"
#define NAME_SEM_SUM "/semsum"
#define NAME_SEM_FIN "/semfin"
#define MAX_LINE_LENGHT 1024
/*prototipi*/
void testMemoria(int *array, int *cont);
bool primo(int n);
void printIntArray(int *array, int size);


int main(int argc, char const *argv[]) {

  if(argc!=3) {
    printf("Uso:\n\t%s nomefile p\n",argv[0]);
    exit(1);
  }

  int p = atoi(argv[2]);
  assert(p>0);

  // ---- creazione array memoria condivisa
  int shm_size = 13*sizeof(int); // numero di byte necessari
  int fd = xshm_open(NAME, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *buffArray = simple_mmap(shm_size, fd, __LINE__,__FILE__);
  int *buffArrayCont = &buffArray[10];
  int *primiSum = &buffArray[11];
  int *fineLettura = &buffArray[12];
  close(fd); // dopo mmap e' possibile chiudere il file descriptor

  // ---- debug provo aree memoria
  //testMemoria(buffArray, buffArrayCont);

  // ---- inizialmente il buffer è vuoto
  *buffArrayCont = 0;

  // ---- inizialmente la somma dei primi e zero
  *primiSum = 0;

  // ---- fine lettura impostato a zero inizialmente
  *fineLettura = 0;

  pthread_mutexattr_t attr;
  // qui sotto andrebbero controllati eventuali errori
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);

  // ---- mutex condiviso per buffer e contatore buffer
  int shMutex_size = sizeof(pthread_mutex_t);
  int fdMutex = xshm_open(NAME_SEM, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fdMutex, shMutex_size, __LINE__, __FILE__);
  pthread_mutex_t *shMutex = simple_mmap(shMutex_size, fdMutex, __LINE__, __FILE__);
  xpthread_mutex_init(shMutex, &attr, __LINE__, __FILE__);
  close(fdMutex);

  // ---- mutex condiviso per variabile primiSum
  int shMutexSum_size = sizeof(pthread_mutex_t);
  int fdMutexSum = xshm_open(NAME_SEM_SUM, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fdMutexSum, shMutexSum_size, __LINE__, __FILE__);
  pthread_mutex_t *shMutexSum = simple_mmap(shMutexSum_size, fdMutexSum, __LINE__, __FILE__);
  xpthread_mutex_init(shMutexSum, &attr, __LINE__, __FILE__);
  close(fdMutexSum);

  // ---- mutex condiviso per variabile fineLettura
  int shMutexFineLettura_size = sizeof(pthread_mutex_t);
  int fdMutexFineLettura = xshm_open(NAME_SEM_FIN, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fdMutexFineLettura, shMutexFineLettura_size, __LINE__, __FILE__);
  pthread_mutex_t *shMutexFineLettura = simple_mmap(shMutexFineLettura_size, fdMutexFineLettura, __LINE__, __FILE__);
  xpthread_mutex_init(shMutexFineLettura, &attr, __LINE__, __FILE__);
  close(fdMutexFineLettura);

  // ---- creazione processi figli
  int flagFiglio;
  pid_t pid;
  for(flagFiglio=0; flagFiglio<p; flagFiglio++ ){
    pid = xfork(__LINE__, __FILE__);
    if(pid==0){break;} // un figlio non deve creare altri figli ...
  }
  if(pid==0){
    /*
    * CODICE FIGLI
    */

    /*debug*/
    printf("[f%d]Ciao\n", flagFiglio);

    int r; // intero letto
    while(true){
      printf("[f%d]Ho intenzione di acquisire il lock di shMutex\n", flagFiglio);
      xpthread_mutex_lock(shMutex, __LINE__, __FILE__);
      printf("[f%d]Ho acquisito il lock di shMutex\n", flagFiglio);
      if(*buffArrayCont==0){
        xpthread_mutex_unlock(shMutex, __LINE__, __FILE__);
        printf("[f%d]Ho rilasciato il lock di shMutex\n", flagFiglio);
        xpthread_mutex_lock(shMutexFineLettura, __LINE__, __FILE__);
        if(*fineLettura == 1){
          xpthread_mutex_unlock(shMutexFineLettura, __LINE__, __FILE__);
          break;
        }
        xpthread_mutex_unlock(shMutexFineLettura, __LINE__, __FILE__);
        printf("[f%d]Non ci sono valori nel buffer, vado a dormire\n", flagFiglio);
        sleep(1);
      }else{
        r = buffArray[(*buffArrayCont)-1];
        printf("[f%d]leggo %d dal buffer dalla posizione %d.\n", flagFiglio, r, *buffArrayCont-1);
        (*buffArrayCont)--;
        xpthread_mutex_unlock(shMutex, __LINE__, __FILE__);
        printf("[f%d]Ho rilasciato il lock di shMutex\n", flagFiglio);
        if(primo(r) == true){
          printf("[f%d] %d è primo.\n", flagFiglio, r);
          xpthread_mutex_lock(shMutexSum, __LINE__, __FILE__);
          (*primiSum)++;
          xpthread_mutex_unlock(shMutexSum, __LINE__, __FILE__);
        }
      }
    }

  }else{
    /*
    * CODICE PADRE
    */

    // ---- apertura file coi numeri
    FILE *f = xfopen(argv[1], "r", __LINE__, __FILE__);
    if(f == NULL){
      perror("Problema con l'apertura del file");
    }

    // ---- predisposizione buffer di lettura
    char *linea = malloc(MAX_LINE_LENGHT);
    if(linea == NULL){
      perror("Problema malloc\n");
      exit(1);
    } // ---- scriviamo tutti zeri nel buffer
    if(memset(linea, 0, MAX_LINE_LENGHT) == NULL){
      perror("Problema con memset()");
      exit(1);
    }

    // -- lettura da file, finchè leggo un numero :
    //  -- leggo un numero dal file
    //  -- tenta di accedere al mutex
    //  -- se buffArrayCont < 10
    //   -- scrive il numero in buffArray[buffArrayCont]
    //   -- buffArrayCont++
    //  -- altrimenti
    //   -- attendo un secondo
    //  rilascio il mutex
    int n;
    size_t size = MAX_LINE_LENGHT;
    bool flag;
    while(getline(&linea, &size, f) != -1){
      n = atoi(linea);
      flag = false;
      printf("[p]leggo %d da file.\n", n);
      while(flag == false){
        printf("[p]ho intenzione di acquisire il lock shMutex.\n");
        xpthread_mutex_lock(shMutex, __LINE__, __FILE__);
        printf("[p]ho acquisito il lock shMutex.\n");
        if(*buffArrayCont<10){
          flag = true;
          buffArray[*buffArrayCont] = n;
          printf("[p]scrivo %d nella posizione %d del buffer\n", n, *buffArrayCont);
          (*buffArrayCont)++;
          xpthread_mutex_unlock(shMutex, __LINE__, __FILE__);
          printf("[p]ho rilasciato il lock shMutex.\n");
        }else{
          printf("[p]variabile buffArrayCont = %d, dormo.\n", *buffArrayCont);
          xpthread_mutex_unlock(shMutex, __LINE__, __FILE__);
          printf("[p]ho rilasciato il lock shMutex.\n");
          sleep(1);
        }
      }
    }

    printf("[p]Lettura terminata, imposto il flag di fine lettura.\n");

    // ---- segnalazione fine
    xpthread_mutex_lock(shMutexFineLettura, __LINE__, __FILE__);
    *fineLettura = 1;
    xpthread_mutex_unlock(shMutexFineLettura, __LINE__, __FILE__);


    // ---- attesa figli
    printf("[p]Lettura terminata, attendo i figli.\n");
    int status;
    for(int i=0; i<p; i++){
      xwait(&status, __LINE__, __FILE__);
      printf("[p]figlio terminato\n");
    }

    printf("La somma dei primi nel file %s e %d.\n", argv[1], *primiSum);

    /*
     * FINE PROGRAMMA DISALLOCAZIONE RISORSE
     */

     // ---- disallocazione risorse ottenute con malloc
     free(linea);

     // ---- chiusura file
     if(fclose(f) != 0){
       perror("Problema chiusura file.");
     }

    // ---- unmap e rimozione memoria condivisa
    xmunmap(buffArray, shm_size, __LINE__, __FILE__);
    xshm_unlink(NAME, __LINE__, __FILE__);

    // ---- distruzione shMutex e sua area memoria condivisa
    xpthread_mutex_destroy(shMutex, __LINE__, __FILE__);
    xmunmap(shMutex, shMutex_size, __LINE__, __FILE__);
    xshm_unlink(NAME_SEM, __LINE__, __FILE__);

    // ---- distruzione shMutexSum e sua area memoria condivisa
    xpthread_mutex_destroy(shMutexSum, __LINE__, __FILE__);
    xmunmap(shMutexSum, shMutexSum_size, __LINE__, __FILE__);
    xshm_unlink(NAME_SEM_SUM, __LINE__, __FILE__);

    // ---- distruzione shMutexSum e sua area memoria condivisa
    xpthread_mutex_destroy(shMutexFineLettura, __LINE__, __FILE__);
    xmunmap(shMutexFineLettura, shMutexFineLettura_size, __LINE__, __FILE__);
    xshm_unlink(NAME_SEM_FIN, __LINE__, __FILE__);

    return 0;
  }
}

void testMemoria(int *array, int *cont){
  printf("test memoria condivisa, array:");
  for(int i = 0; i < 10; i++){
    array[i] = i;
  }
  *cont = 15;
  printIntArray(array, 10);
  printf("test memoria condivisa, contatore: %d\n", *cont);
}


// restituisce true/false a seconda che n sia primo o composto
bool primo(int n)
{
  int i;
  if(n<2) return false;
  if(n%2==0) {
    if(n==2)
      return true;
    else
      return false;
  }
  for (i=3; i*i<=n; i += 2) {
      if(n%i==0) {
          return false;
      }
  }
  return true;
}

/* stampa array interi */
void printIntArray(int *array, int size){
  for(int i = 0; i < size; i++){
    printf("[%d]", array[i]);
  }
  printf("\n");
}
