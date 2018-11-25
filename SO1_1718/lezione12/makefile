# makefile che utilizza regole implicite e variabili
#
# usare 
#    make <nome_eseguibile> 
# per compilare un unico programma, o semplicemente
#    make
# per compilarli tutti 
#
CFLAGS=-Wall -std=c99 -g -O
LDFLAGS=-lpthread
CC=gcc

# eseguibili da creare
EXECS=esercizio


all: $(EXECS)


# esercizio
esercizio: esercizio.o  xerrors.o
	$(CC) $(CFLAGS) -o esercizio esercizio.o xerrors.o $(LDFLAGS)

esercizio.o: esercizio.c  xerrors.h
	$(CC) $(CFLAGS) -o esercizio.o -c esercizio.c