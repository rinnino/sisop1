#define _GNU_SOURCE   // permette di usare estensioni GNU
#include <stdio.h>    // permette di usare scanf printf .. 
#include <stdlib.h>   // conversioni stringa/numero rand() abs() exit() etc ...
#include <stdbool.h>  // gestisce tipo bool (per variabili booleane)
#include <assert.h>   // permette di usare la funzione assert
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

// prototipi di xerrori.c

// stampa messaggio d'errore e termina processo
void die(const char *s); 

// operazioni su file  
FILE *xfopen(const char *pathname, const char *mode, const char *file, const int line);

// operazioni su processi
pid_t xfork(const char *file, const int linea);
pid_t xwait(int *status, const char *file, const int linea);
// pipe
int xpipe(int pipefd[2], const char *file, const int linea);

