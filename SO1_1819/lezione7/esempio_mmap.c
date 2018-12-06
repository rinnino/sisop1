#include "xerrors.h"



int main(int argc,char *argv[])
{
  if(argc!=2) { 
    fprintf(stderr,"Uso\n\t%s nome_file\n", argv[0]);
    exit(1);
  }
  
  // ----  mapping 
  int fd = open(argv[1],O_RDWR, 0600);
  if(fd<0) die("Error open");
  int size = lseek(fd,0,SEEK_END); // ottiene dimensione file 
  char *a = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  if(*a<=0) perror("mmap");
  close(fd); // dopo mmap e' possibile chiudere il file descriptor
  for(int i=0;i<size;i++) {
    if(a[i]=='a') a[i]='e';
  }
  
  // unmap e termina
  xmunmap(a,size,__LINE__, __FILE__);
  return 0;
}
