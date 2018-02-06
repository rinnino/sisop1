#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LENGHT_NOME 20

int main(int argc, char *argv[]){
    //variabili
    int n; // contiene la sommatoria
    int b; // buffer lettura
    char nome[MAX_LENGHT_NOME];
    double time_spent;
    clock_t begin, end;

    //controllo input da console
    if(argc!=2){
        perror("numero argomenti non valido");
        exit(1);
    }

    //inizializziamo n e nome
    n = 0;
    // controllo input nome
    if(strlen(argv[1]) >= MAX_LENGHT_NOME){
        perror("nome file troppo lungo");
        exit(1);
    }
    strcpy(nome, argv[1]);

    //timer start
    begin = clock();

    // gli argomenti sulla linea di comando sono in n e nome
    // apre file in scrittura
    FILE * f = fopen(nome,"rb");
    if(f==NULL) {  // se il file non è stato aperto visualizza messaggio d'errore e esci
    perror("Errore apertura file");
    exit(1);
    }

    // legge valori sul file e sommatoria in n
    while(1) {
        int e = fread(&b, sizeof(int), 1, f); // legge da f
        if (e==0){
            break;
        }
        if(e!=1) {
            perror("Errore lettura file");
            exit(1);
        }
        //sommatoria
        n += b;

        // debug
        //printf("debug: [n=%d] [e=%d]\n", n, e);
    }
    // chiude file 
    int e = fclose(f);
    if(e!=0) {
    perror("Errore chiusura file");
    exit(1);
    }

    // timer stop
    end = clock();

    // determiniamo i secondi trascorsi
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    // stampa tempo esecuzione
    printf("lettura file in %lf secondi\n", time_spent);

    printf("sommatoria: %d\n", n);

}