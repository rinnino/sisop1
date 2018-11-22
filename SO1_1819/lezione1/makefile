# definizione variabili
CFLAGS=-Wall -O -g -std=c99
CC=gcc

# elenco eseguibili
EXECS=scrivi leggi

all: $(EXECS)

leggi: leggi.o xerrori.o
	$(CC) $(CFLAGS) -o $@ $^

scrivi: scrivi.o xerrori.o
	$(CC) $(CFLAGS) -o $@ $^

# regola generale per i file oggetto
%.o: %.c xerrori.h
	$(CC) $(CFLAGS) -o $@ -c $<

# utile per il docente per creare un archvio da mettere su DIR
tarfile:
	tar -zcf lezione.tgz makefile *.c *.h

# per quando si vuole ricominciare da zero
clean:
	rm -f *.o $(EXECS)
