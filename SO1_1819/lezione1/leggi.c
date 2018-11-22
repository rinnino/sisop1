#include "xerrori.h"

int main(int argc, char const *argv[]) {
  /* variabili */
  int buff=0;
  int sum=0;

  /* controllo argv */
  if(argc != 2){
    fprintf(stderr, "Uso:\n\t%s nomefile\n\n", argv[0]);
    exit(1);
  }
  /* apriamo file in lettura */
  FILE *f = xfopen(argv[1], "rb", __FILE__, __LINE__);

  /* lettura */
  size_t e;
  while(true){
    e = fread(&buff, sizeof(int), 1, f);
    if(e!=1){
      break;
    }
    sum += buff;
  }

  /* output somma */
  printf("La somma vale %d\n", sum);

  /* chiusura file */
  int e = fclose(f);
  if(e!=0){
    perror("Errore chiusura file\n");
    exit(1);
  }

  return 0;
}
