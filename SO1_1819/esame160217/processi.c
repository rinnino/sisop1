#include "xerrors.h"

// nomi dei file in memoria condivisa
#define SHM_NUMERI "/shm_numeri"
#define SHM_BUFFPC "/shm_buffpc"
#define SHM_SEM_T "/shm_sem_t"
#define SHM_MUTEX "/shm_mutex"
#define SHM_CINDEX "/shm_cindex"

//dimensione buffer pc
#define Buf_size 10
#define MAX_FILE_NAME 100

//elevazione a potenza x^y, valida solo per y>=0
long pot(int x, int y);

int main(int argc, char const *argv[]) {
  if(argc!=4) {
    fprintf(stderr,"Uso\n\t%s nomefile M N\n", argv[0]);
    exit(1);
  }
  int nProcessi = atoi(argv[3]);
  int maxExp = atoi(argv[2]);

  //apertura file
  FILE *f = xfopen(argv[1], "r", __LINE__, __FILE__);
  if(f == NULL){
    perror("Problema con l'apertura del file");
  }

  //contiamo quanti interi ci sono
  int nIntFile=0;
  int n;
  while(true){
    int e  = fscanf(f,"%d",&n);
    if(e!=1) break;
    nIntFile++;
  }
  printf("[p]Ci sono %d interi nel file %s\n", nIntFile, argv[1]);

  // ---- creazione array di interi in memoria condivisa
  int shm_size = nIntFile*sizeof(int); // numero di byte necessari
  int fd = xshm_open(SHM_NUMERI,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *a = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor

  //riavvolgiamo il file al punto di inizio
  rewind(f);

  //popoliamo l'array condiviso

  for(int i=0;;i++){
    int e  = fscanf(f,"%d",&n);
    if(e!=1) break;
    a[i]=n;
  }

  //stampa debug
  printf("[p]Array condiviso: ");
  for(int i=0; i<nIntFile; i++){
    printf("[%d]", a[i]);
  }
  printf("\n");

  //il file non mi serve piu, lo chiudo.
  fclose(f);

  // ---- creazione array condiviso semafori per buffer pc
  shm_size = 2*sizeof(sem_t); // numero di byte necessari
  fd = xshm_open(SHM_SEM_T,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  sem_t *semaforiPC = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor

  sem_t *sem_free_slots = &semaforiPC[0];
  sem_t *sem_data_items = &semaforiPC[1];

  //init semafori condivisi
  xsem_init(sem_free_slots, 1, Buf_size, __LINE__, __FILE__);
  xsem_init(sem_data_items, 1, 0, __LINE__, __FILE__);

  // ---- creazione array condiviso mutex per buffer pc
  shm_size = sizeof(pthread_mutex_t); // numero di byte necessari
  fd = xshm_open(SHM_MUTEX,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  pthread_mutex_t *mutexPC = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor

  // init mutex condiviso
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
  xpthread_mutex_init(mutexPC, &attr, __LINE__, __FILE__);

  // ---- creazione  buffer PC condiviso
  shm_size = maxExp*sizeof(int); // numero di byte necessari
  fd = xshm_open(SHM_BUFFPC, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *buffpc = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor

  // ---- creazione  cindex condiviso
  shm_size = sizeof(int); // numero di byte necessari
  fd = xshm_open(SHM_CINDEX, O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *cindex = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor
  *cindex = 0;

  // ---- creazione processi figli
  int flagFiglio;
  pid_t pid=0;
  for(flagFiglio=0; flagFiglio<nProcessi; flagFiglio++ ){
    pid = xfork(__LINE__, __FILE__);
    if(pid==0){break;} // un figlio non deve creare altri figli ...
  }
  if(pid==0){
    /*
    * CODICE FIGLI
    */
    printf("[f%d]Ciao!\n", flagFiglio);
    char nomefile[MAX_FILE_NAME];
    FILE *f;

    //consumatore: preleva un esponente i e crea nomefile.i
    int exp;
    while(true){
      xsem_wait(sem_data_items, __LINE__, __FILE__);
      xpthread_mutex_lock(mutexPC, __LINE__, __FILE__);
      exp=buffpc[*cindex%Buf_size];
      (*cindex)+=1;
      xpthread_mutex_unlock(mutexPC, __LINE__, __FILE__);
      xsem_post(sem_free_slots, __LINE__, __FILE__);
      printf("[f%d]Ho prelevato l'esponente %d\n", flagFiglio, exp);
      if(exp==-1){break;} //fine consumatore

      //apertura file
      sprintf(nomefile, "%s.%d", argv[1], exp);
      f = xfopen(nomefile, "w", __LINE__, __FILE__);
      if(f == NULL){
        perror("Problema con l'apertura del file");
      }
      //scrittura numeri
      for(int i=0; i<nIntFile; i++){
        fprintf(f, "%ld\n", pot(a[i], exp));
      }

      //chiusura file
      fclose(f);
    }
    printf("[f%d]Ho terminato.\n", flagFiglio);
  }else{
    /*
    * CODICE PADRE
    */

    //il padre è produttore e le unità di lavoro sono gli esponenti da 1 ad M
    //l'esponente -1 segnala la fine del produttore
    int pindex=0;
    for(int i=1; i<=maxExp; i++){
      xsem_wait(sem_free_slots, __LINE__, __FILE__);
      xpthread_mutex_lock(mutexPC, __LINE__, __FILE__);
      buffpc[pindex%Buf_size] = i;
      pindex+=1;
      xpthread_mutex_unlock(mutexPC, __LINE__, __FILE__);
      xsem_post(sem_data_items, __LINE__, __FILE__);
      printf("[p]Ho prodotto l'esponente %d\n", i);
    }
    //segnala terminazione
    for(int i=0; i<nProcessi; i++){
      xsem_wait(sem_free_slots, __LINE__, __FILE__);
      xpthread_mutex_lock(mutexPC, __LINE__, __FILE__);
      buffpc[pindex%Buf_size] = -1;
      pindex+=1;
      xpthread_mutex_unlock(mutexPC, __LINE__, __FILE__);
      xsem_post(sem_data_items, __LINE__, __FILE__);
    }
    //attesa figli
    int status;
    int figliChiusi=0;
    for(; figliChiusi<nProcessi; figliChiusi++){
      xwait(&status, __LINE__, __FILE__);
    }
    printf("[p]Ho terminato %d figli\n", figliChiusi);

    //distruzione semafori e mutex
    int e = sem_destroy(sem_free_slots);
    assert(e==0);
    e = sem_destroy(sem_data_items);
    assert(e==0);
    xpthread_mutex_destroy(mutexPC , __LINE__, __FILE__);

    //detach memoria condivisa
    // unmap e rimozione memoria condivisa
    xmunmap(a,nIntFile*sizeof(int),__LINE__, __FILE__);
    xmunmap(semaforiPC,2*sizeof(sem_t),__LINE__, __FILE__);
    xmunmap(mutexPC,sizeof(pthread_mutex_t),__LINE__, __FILE__);
    xmunmap(buffpc,maxExp*sizeof(int),__LINE__, __FILE__);
    xmunmap(cindex,sizeof(int),__LINE__, __FILE__);
    xshm_unlink(SHM_NUMERI,__LINE__, __FILE__);
    xshm_unlink(SHM_SEM_T,__LINE__, __FILE__);
    xshm_unlink(SHM_MUTEX,__LINE__, __FILE__);
    xshm_unlink(SHM_BUFFPC,__LINE__, __FILE__);
    xshm_unlink(SHM_CINDEX,__LINE__, __FILE__);


  }
  return 0;
}

long pot(int x, int y){
  if(y==0) return 1;
  if(y==1) return x;
  long ris = x;
  for(int i=1; i<y; i++){ris=ris*x;}
  return ris;
}
