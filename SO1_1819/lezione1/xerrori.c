#include "xerrori.h"

FILE *xfopen(const char *pathname, const char *mode, const char *file, const int line) {
  FILE *f = fopen(pathname,mode);
  if(f==NULL) {
    perror("Errore apertura file");
    fprintf(stderr,"(%s:%d)\n",file,line);
    exit(1);
  }
  return f;
}

size_t xfwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream,
                const char *file, const int linea){
      size_t e = fwrite(ptr, size, nmemb, stream);
      if(e!=nmemb){
        perror("Errore scrittura file");
        fprintf(stderr, "(%s:%d\n)",file,linea );
        exit(1);
      }
      return e;
}
