#include "xerrori.h"

// xerrori.c
// collezione di chiamate a funzioni di sistema con controllo output
// i prototipi sono in xerrori.h




void die(const char *s) {
  perror(s);
  exit(1);
}

FILE *xfopen(const char *pathname, const char *mode, const char *file, const int line) {
  FILE *f = fopen(pathname,mode);
  if(f==NULL) {
    perror("Errore apertura file");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    exit(1);
  }
  return f;
}


// processi 

pid_t xfork(const char *file, const int line) {
  pid_t p = fork();
  if(p<0) {
    perror("Errore chiamata fork");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    exit(1);
  }
  return p;
}

pid_t xwait(int *status, const char *file, const int line)
{
  pid_t p = wait(status);
  if(p<0) {
    perror("Errore chiamata wait");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    exit(1);
  }
  return p;
}

int xpipe(int pipefd[2], const char *file, const int line) {
  int e = pipe(pipefd);
  if(e!=0) {
    perror("Errore creazione pipe"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),line,file);
    exit(1);
  }
  return e;
}
