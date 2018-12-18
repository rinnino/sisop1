# makefile che utilizza regole implicite e variabili
#
# usare
#    make <nome_eseguibile>
# per compilare un unico programma, o semplicemente
#    make
# per compilarli tutti
#
CFLAGS=-Wall -std=c99 -g -O
LDFLAGS=-pthread -lrt
CC=gcc

# eseguibili da creare
EXECS=es1 es2

all: $(EXECS)

# regola per gli eseguibili
es%: es%.o xerrors.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# regola generale per i file oggetto
%.o: %.c xerrors.h
	$(CC) $(CFLAGS) -o $@ -c $<

# xerrors.o
xerrors.o: xerrors.c  xerrors.h
	$(CC) $(CFLAGS) -o xerrors.o -c xerrors.c


# utile per il docente per creare un archvio da mettere su DIR
tarfile:
	tar -zcf lezione.tgz makefile *.c *.h


clean:
	rm *.o $(EXECS)
