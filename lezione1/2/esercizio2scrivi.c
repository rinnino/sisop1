#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    //variabili
    int n;
    char nome[20];
    int f; //file descriptor

    //inizializziamo n e nome
    n = atoi(argv[1]);
    strcpy(nome, argv[2]);

    // gli argomenti sulla linea di comando sono in n e nome
    // apre file in scrittura
    f = open(nome, O_WRONLY|O_CREAT|O_APPEND, 0666);
    if(f<0) {  // se il file non è stato aperto visualizza messaggio d'errore e esci
        perror("Errore apertura file");
        exit(1);
    }

    // scrive valori sul file
    for(int i=1;i<=n;i++) {
        int e = write(f, &i, sizeof(int)); // scrive su f il contenuto di i 
        if(e!=sizeof(int)) {
            perror("Errore scrittura file");
            exit(1);
        }
    }
    // chiude file 
    int e = close(f);
    if(e!=0) {
    perror("Errore chiusura file");
    exit(1);
    }
}