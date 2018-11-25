# definizione variabili
CFLAGS=-Wall -O -g -std=c99
CC=gcc

# elenco eseguibili
EXECS=contaprimi contaprimipipe contaprimifile pariDispariPipe

all: $(EXECS)

pariDispariPipe: pariDispariPipe.o xerrori.o
	$(CC) $(CFLAGS) -o $@ $^

contaprimipipe: contaprimipipe.o xerrori.o
	$(CC) $(CFLAGS) -o $@ $^

contaprimifile: contaprimifile.o xerrori.o
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
