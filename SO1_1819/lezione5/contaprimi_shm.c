#include "xerrors.h"


// conteggio dei primi con piu' processi utilizzando
// un array condiviso e nessun semaforo


#define Nome "/contaprimi"


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


  // ---- creazione array memoria condivisa
  int shm_size = n*sizeof(int); // numero di byte necessari
  int fd = xshm_open(Nome,O_RDWR | O_CREAT, 0600,__LINE__,__FILE__);
  xftruncate(fd, shm_size, __LINE__,__FILE__);
  int *a = simple_mmap(shm_size,fd, __LINE__,__FILE__);
  close(fd); // dopo mmap e' possibile chiudere il file descriptor
  
  // creazione processi figlio
  for(int i=0; i<n; i++) {
    pid_t pid= xfork(__LINE__, __FILE__);
    if(pid==0) { //processo figlio
      int conta=0;
      for(int j=1+2*i; j< m; j+=2*n)
         if(primo(j)) conta++;
      // scrivo il valore di conta nella memoria condivisa
      a[i]= conta;
      // unmap memoria condivisa
      xmunmap(a,shm_size,__LINE__, __FILE__);
      fprintf(stderr,"Processo %d terminato dopo avere trovato %d primi\n",i,conta);
      exit(0);
    }
  }
  // codice processo padre
  // aspetta che abbiano finito i figli: 
  for(int i=0; i<n; i++)  xwait(NULL, __LINE__, __FILE__);
    
  // calcola e restituisce il risultato 
  int conta = 0;
  for(int i=0; i<n; i++) conta += a[i];
  printf("Numero primi dispari tra 1 e %d (escluso): %d\n",m,conta);
  
  // unmap e rimozione memoria condivisa
  xmunmap(a,shm_size,__LINE__, __FILE__);
  xshm_unlink(Nome,__LINE__, __FILE__);
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
