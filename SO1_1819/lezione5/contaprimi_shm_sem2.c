#include "xerrors.h"

// conteggio dei primi con piu' processi utilizzando
// un intero condiviso e 2 semafori

// nomi dei file in memoria condivisa
#define Nome "/contaprimi_int"
#define Nome1 "/contaprimi_sem"


//Prototipi
bool primo(int n);

int main(int argc,char *argv[])
{
  if(argc!=3) {
    fprintf(stderr,"Uso\n\t%s m num_processi\n", argv[0]);
    exit(1);
  }
  // conversione input
  int m= atoi(argv[1]);
  if(m<1) die("limite primi non valido");
  int n= atoi(argv[2]);
  if(n<0) die("numero di processi non valido");


  // ---- creazione array di interi in memoria condivisa
  int shm_size = 1*sizeof(int); // numero di byte necessari
  int fd = xshm_open(Nome,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *a = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd);  // dopo mmap e' possibile chiudere il file descriptor
  a[0]=0; // numero totale di primi

  // ---- memoria condivisa per i semafori
  int shm_size1 = 2*sizeof(sem_t); // numero di byte necessari
  int fd1 = xshm_open(Nome1,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd1, shm_size1, __LINE__,__FILE__);
  sem_t *sem = simple_mmap(shm_size1,fd1, __LINE__,__FILE__);
  close(fd1); // dopo mmap e' possibile chiudere il file descriptor
  sem_t *semconta = &sem[0];  // oppure  = sem 
  sem_t *semfinito = &sem[1]; // oppure = sem+1
  xsem_init(semconta,1,1,__LINE__, __FILE__);
  xsem_init(semfinito,1,0,__LINE__, __FILE__);
  
  // creazione processi figlio
  for(int i=0; i<n; i++) {
    pid_t pid= xfork(__LINE__, __FILE__);
    if(pid==0) { //processo figlio
      int conta=0;
      // con questo ciclo for i processi figli testano numeri diversi 
      for(int j=1+2*i; j< m; j+=2*n)
         if(primo(j)) conta++;
      // scrivo il valore di conta nella memoria condivisa
      xsem_wait(semconta,__LINE__, __FILE__);
      a[0] += conta; // accesso esclusivo
      xsem_post(semconta,__LINE__, __FILE__);
      // segnalo al padre che ho terminato 
      xsem_post(semfinito,__LINE__, __FILE__);
      // unmap memoria condivisa
      xmunmap(a,shm_size,__LINE__, __FILE__);
      xmunmap(sem,shm_size1,__LINE__, __FILE__);
      fprintf(stderr,"Processo %d terminato dopo avere trovato %d primi\n",i,conta);
      exit(0);
    }
  }
  // codice processo padre
  
  // voglio accorgermi che il risultato in a[0] sia pronto senza dovere
  // eseguire al wait, che richiede a terminazione dei figli

  // decremento n volte il semaforo semfinito
  // ci riesco solo quando tutti i figli hanno finito
  for(int i=0;i<n;i++)
    xsem_wait(semfinito, __LINE__, __FILE__);

  // restituisce il risultato 
  int conta = a[0];
  printf("Numero primi dispari tra 1 e %d (escluso): %d\n",m,conta);
  
  // unmap e rimozione memoria condivisa
  xmunmap(a,shm_size,__LINE__, __FILE__);
  xmunmap(sem,shm_size1,__LINE__, __FILE__);
  xshm_unlink(Nome,__LINE__, __FILE__);
  xshm_unlink(Nome1,__LINE__, __FILE__);
  return 0;
}


bool primo(int n)
{
    int i;
    if(n<2) 
        return false;
    if(n%2==0) 
    {
        if(n==2)
            return true;
        else
            return false;
    }
    for(i=3; i*i<=n; i+= 2) 
    {
        if(n%i==0) 
        {
            return false;
        }
    }
    return true;
}
