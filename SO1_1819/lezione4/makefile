# definizione variabili
CFLAGS=-Wall -O -g -std=c99
CC=gcc

# elenco eseguibili
EXECS=sortPipe esercizio2

all: $(EXECS)

sortPipe: sortPipe.o xerrori.o
	$(CC) $(CFLAGS) -o $@ $^

esercizio2: esercizio2.o xerrori.o
	$(CC) $(CFLAGS) -o $@ $^

# regola generale per i file oggetto
%.o: %.c xerrori.h
	$(CC) $(CFLAGS) -o $@ -c $<

# utile per il docente per creare un archvio da mettere su DIR
tarfile:
	tar -zcf lezione.tgz makefile *.c *.h *.py

# per quando si vuole ricominciare da zero
clean:
	rm -f *.o $(EXECS)
