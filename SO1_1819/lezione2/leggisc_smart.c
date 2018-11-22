#include "xerrori.h"

int main(int argc, char const *argv[]) {
  /* variabili */
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

  /*determinazione dimensione file */
  off_t dimFile = lseek(fd, 0, SEEK_END);
  if(dimFile == -1){
    perror("Problema determinazione dimensione del file");
    exit(1);
  }
  printf("dimensione file %s : %ld byte\n", argv[1], dimFile);

  /* dobbiamo riposizionare il seek all'inizio del file */
  if(lseek(fd, 0, SEEK_SET) != 0){
    perror("Problema riposizionamento offset con lseek");
    exit(1);
  }

  /* creazione buffer */
  /*dimFile è la dimensione del file in byte*/
  /*dimBuf è la dimensione del file in "int" arrotondata per eccesso*/
  long dimBuf = 0;
  if((dimFile%(sizeof(int))) == 0){
    dimBuf = dimFile/sizeof(int);
  }else{
    dimBuf = dimFile/sizeof(int)+1;
  }
  int buf[dimBuf];
  /* puliamo il buffer con tutti zero*/
  if(memset(buf, 0, dimFile) == NULL){
    perror("Problema con memset()");
    exit(1);
  }
  printf("Buffer allocato, %ld numeri interi\n", dimBuf);

  /* lettura da file */
  ssize_t er;
  er = read(fd, &buf, dimFile);
  if(er!=dimFile){
    printf("ritorno read: %ld\n", er);
    fprintf(stderr, "Problema read()\n");
    exit(1);
  }

  /* calcolo somma da buffer */
  for(int i = 0; i < dimBuf; i++){
    sum += buf[i];
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
