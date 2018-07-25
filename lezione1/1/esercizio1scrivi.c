#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define MAX_LENGHT_NOME 20

int main(int argc, char *argv[]){
    //variabili
    int n; // numero max
    char nome[MAX_LENGHT_NOME]; //nome del file
    double time_spent;
    clock_t begin, end;

    //controllo input da console
    if(argc!=3){
        fprintf(stderr, "numero argomenti non valido");
        exit(1);
    }

    //inizializziamo n e nome
    n = atoi(argv[1]);
    // controllo input nome
    if(strlen(argv[2]) >= MAX_LENGHT_NOME){
        fprintf(stderr, "nome file troppo lungo\n");
        exit(1);
    }
    strcpy(nome, argv[2]);

    //timer start
    begin = clock();
    if(begin<0){
        perror("Problema clock()");
    }

    // gli argomenti sulla linea di comando sono in n e nome
    // apre file in scrittura
    FILE * f = fopen(nome,"wb");
    if(f==NULL) {  // se il file non è stato aperto visualizza messaggio d'errore e esci
        perror("Errore apertura file");
        exit(1);
    }

    // scrive valori sul file
    for(int i=1;i<=n;i++) {
        int e = fwrite(&i, sizeof(int), 1, f); // scrive su f il contenuto di i 
        if(e!=1) {
            perror("Errore scrittura file");
            exit(1);
        }
    }
    // chiude file 
    int e = fclose(f);
    if(e!=0) {
    perror("Errore chiusura file");
    exit(1);
    }

    // timer stop
    end = clock();
    if(end<0){
        perror("Problema clock()");
    }

    // determiniamo i secondi trascorsi
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    // stampa tempo esecuzione
    printf("scrittura file in %lf secondi\n", time_spent);
}