#include "xerrori.h"

int main(int argc, char const *argv[]) {
  /* variabili */
  int buff=0;
  unsigned long long sum = 0;

  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s nomefile\n\n", argv[0]);
    exit(1);
  }
  /* apriamo file in lettura */
  int fd = open(argv[1], O_RDONLY);
  if(fd==-1){
    perror("Problema con l'apertura del file");
  }

  /* lettura */
  ssize_t er;
  while(true){
    er = read(fd, &buff, sizeof(int));
    if(er!=sizeof(int)){
      break;
    }
    sum += buff;
  }

  /* output somma */
  printf("La somma vale %llu\n", sum);

  /* chiusura file */
  int ec = close(fd);
  if(ec==-1){
    perror("Errore chiusura file\n");
    exit(1);
  }

  return 0;
}
