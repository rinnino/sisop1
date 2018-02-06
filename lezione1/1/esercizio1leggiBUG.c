#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    //variabili
    int n; // contiene la sommatoria
    int b; // buffer lettura
    int test; // per il test dell'EOF
    char nome[20];

    //inizializziamo n e nome
    n = 0;
    strcpy(nome, argv[1]);

    // gli argomenti sulla linea di comando sono in n e nome
    // apre file in scrittura
    FILE * f = fopen(nome,"rb");
    if(f==NULL) {  // se il file non è stato aperto visualizza messaggio d'errore e esci
    perror("Errore apertura file");
    exit(1);
    }

    // legge valori sul file e sommatoria in n
    while(fscanf (f, "%d", &test) != EOF) {
        int e = fread(&b, sizeof(int), 1, f); // legge da f
        if(e!=1) {
            perror("Errore lettura file");
            exit(1);
        }
        //sommatoria
        n += b;
        printf("debug: [n=%d] [e=%d] [test=%d]", n, e, test);
    }
    // chiude file 
    int e = fclose(f);
    if(e!=0) {
    perror("Errore chiusura file");
    exit(1);
    }
    printf("sommatoria: %d\n", n);
}