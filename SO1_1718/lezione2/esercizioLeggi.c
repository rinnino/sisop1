#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define MAX_LENGHT_NOME 20

int sumBuff(int *buff, int dimBuff);

int main(int argc, char *argv[]){
    //variabili
    int dimbuff = atoi(argv[2]);
    int n; // contiene la sommatoria
    int f; //file descriptor
    char nome[MAX_LENGHT_NOME];
    double time_spent;
    clock_t begin, end;
    int buff[dimbuff];
    int i = 0; //conta loop

    //controllo input da console
    if(argc!=3){
        fprintf(stderr, "numero argomenti non valido");
        exit(1);
    }

    // inizializziamo n e nome
    n = 0;
    // controllo input nome
    if(strlen(argv[1]) >= MAX_LENGHT_NOME){
        fprintf(stderr, "nome file troppo lungo");
        exit(1);
    }
    strcpy(nome, argv[1]);

    //controllo dimensione buffer
    if(dimbuff<=0){
        printf("dimensione buffer non valida\n");
        exit(1);
    }

    //timer start
    begin = clock();
    if(begin<0){
        perror("Problema clock()");
    }

    // gli argomenti sulla linea di comando sono in n e nome
    // apre file in scrittura
    f = open(nome, O_RDONLY);
    if(f<0) {  // se il file non è stato aperto visualizza messaggio d'errore e esci
        perror("Errore apertura file");
        exit(1);
    }

    // legge valori sul file e sommatoria in n
    while(1) {
        //scrivo tutti zeri nel buffer, per risolvere del bug se
        //la dimensione del buffer non è divisore.
        //for(int i=0; i<dimbuff; i++){
        //    buff[i] = 0;
        //}

        memset(buff, 0, dimbuff*sizeof(int));

        int e = read(f, buff, dimbuff*sizeof(int)); // legge da f
        if(e == 0) {
            break;
        }
        if(e<0){
            // errore i/o
            perror("Errore lettura file");
            exit(1);
        }

        //sommatoria
        n += sumBuff(buff, dimbuff);

        //debug
        printf("sommatoria ciclo %d = %d\n", i, n);
        i++;
    }
    // chiude file 
    int e = close(f);
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
    printf("lettura file in %lf secondi\n", time_spent);

    printf("sommatoria: %d\n", n);
}

//sommatoria numeri nel buffer
int sumBuff(int buff[], int dimBuff){
    int sum = 0;
    for(int i=0; i<dimBuff; i++){
        sum += buff[i];
    }
    return sum;
}