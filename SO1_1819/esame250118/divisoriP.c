#include "xerrors.h"

// nomi dei file in memoria condivisa
#define SHM_SEM "/shm_sem_file"

//restituisce il numero di divisori di n
int divisori(int n);

int main(int argc, char const *argv[]) {
  //controllo input
  if(argc!=4) {
    printf("Uso\n\t%s infile outfile nump\n", argv[0]);
    exit(1);
  }

  int nProcessi = atoi(argv[3]);
  assert(nProcessi>0);

  //creiamo la pipe
  int p[2];
  int e = pipe(p);
  if(e>0) die("Problema creazione pipeUp");

  //creiamo mutex per i file condiviso fra processi
  int shm_size = sizeof(pthread_mutex_t); // numero di byte necessari
  int fd = xshm_open(SHM_SEM,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  pthread_mutex_t *mutex_file = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor

  // init mutex condiviso
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
  xpthread_mutex_init(mutex_file, &attr, __LINE__, __FILE__);

  //apertura file output
  FILE *f_out = xfopen(argv[2], "wb", __LINE__, __FILE__);

  // ---- creazione processi figli
  int flagFiglio;
  pid_t pid;
  for(flagFiglio=0; flagFiglio<nProcessi; flagFiglio++ ){
    pid = xfork(__LINE__, __FILE__);
    if(pid==0){break;} // un figlio non deve creare altri figli ...
  }
  if(pid==0){
    /*
    * CODICE FIGLI
    */
    //chiudiamo i lati delle pipe che non ci servono
    close(p[1]); //sola lettura

    //consumatore
    int n;
    ssize_t er;
    while(true){
      er = read(p[0], &n, sizeof(int));
      if(er == 0){break;} //fine pipe
      if(er!= sizeof(int)){
        //errore lettura
        die("[fd]Problema read pipeDown");
      }
      //debug
      printf("[f%d]Leggo %d dalla pipe \n", flagFiglio, n);
      //scrivo su file in modalità binaria
      xpthread_mutex_lock(mutex_file, __LINE__, __FILE__);
      fwrite(&n, sizeof(int), 1, f_out); // numero
      n = divisori(n);
      fwrite(&n, sizeof(int), 1, f_out); // divisore del numero
      xpthread_mutex_unlock(mutex_file, __LINE__, __FILE__);
    }
    close(p[0]);
  }else{
    /*
    * CODICE PADRE
    */

    //chiudiamo i lati delle pipe che non ci servono
    close(p[0]); //sola scrittura

    //produttore
    int e, n;
    ssize_t ew;
    FILE *f_in = xfopen(argv[1], "r", __LINE__, __FILE__);
    while(true){
      e = fscanf(f_in, "%d", &n);
      if(e!=1){break;} // se il file non ha piu numeri esci
      printf("[p]Leggo %d da %s\n", n, argv[1]);
      //produco scrivendo sulla pipe
      ew = write(p[1], &n, sizeof(int));
      if(ew != sizeof(int)){
        perror("[p]Problema con la write su p");
        exit(1);
      }
    }
    //chiusura pipe e file input
    close(p[1]);
    fclose(f_in);

    // ---- attesa figli
    printf("[p]Procedura terminata, attendo i figli.\n");
    int status;
    int figliChiusi=0;
    for(; figliChiusi<nProcessi; figliChiusi++){
      xwait(&status, __LINE__, __FILE__);
      //printf("[p]figlio terminato\n");
    }
    printf("[p]Ho terminato i %d figli\n", figliChiusi);

    //disallocazione semaforo condiviso
    xmunmap(mutex_file, shm_size,__LINE__, __FILE__);
    xshm_unlink(SHM_SEM,__LINE__, __FILE__);

    //chiusura file output
    fclose(f_out);
  }
  /*
  * CODICE PADRE E FIGLIO
  */
  return 0;
}

int divisori(int n){
  int div = 2;
  for(int i=2;i<n;i++)
    if(n%i==0) div++;
  return div;
}
