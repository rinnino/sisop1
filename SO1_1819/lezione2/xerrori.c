#include "xerrori.h"


void die(const char *s) {
  perror(s);
  exit(1);
}

FILE *xfopen(const char *pathname, const char *mode, const char *file, const int line) {
  FILE *f = fopen(pathname,mode);
  if(f==NULL) {
    perror("Errore apertura file");
    fprintf(stderr,"(%s:%d)\n",file,line);
    exit(1);
  }
  return f;
}

pid_t xfork(const char *file, const int line) {
  pid_t p = fork();
  if(p<0) {
    perror("Errore chiamata fork");
    fprintf(stderr,"(%s:%d)\n",file,line);
    exit(1);
  }
  return p;
}

pid_t xwait(int *status, const char *file, const int line)
{
  pid_t p = wait(status);
  if(p<0) {
    perror("Errore chiamata wait");
    fprintf(stderr,"(%s:%d)\n",file,line);
    exit(1);
  }
  return p;
}
