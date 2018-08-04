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
EXECS=semafori sommeT


all: $(EXECS)


# somma_primi
semafori: semafori.o  xerrors.o
	$(CC) $(CFLAGS) -o semafori semafori.o xerrors.o $(LDFLAGS)

semafori.o: semafori.c  xerrors.h
	$(CC) $(CFLAGS) -o semafori.o -c semafori.c

# sommeT
sommeT: sommeT.o  xerrors.o
	$(CC) $(CFLAGS) -o sommeT sommeT.o xerrors.o $(LDFLAGS)

sommeT.o: sommeT.c  xerrors.h
	$(CC) $(CFLAGS) -o sommeT.o -c sommeT.c