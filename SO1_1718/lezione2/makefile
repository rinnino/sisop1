# makefile che utilizza regole implicite  e variabili
#
# In questo esempio sfruttiamo la presenza della regola implicita
# che per ottenere n.o da n.c esegue il comando  
#   $(CC) -c $(CFLAGS) -o n.o n.c

# definizione variabili
CFLAGS=-Wall -std=c99 -g -O
CC=gcc

all: lezione2 clean

# l'esercizio e composto da 2 programmi
lezione2: esScrivi esLeggi

# file eseguibile scrivi
esScrivi: esercizioScrivi.o
	$(CC) $(CFLAGS) -o esercizioScrivi esercizioScrivi.o

# file oggetto scrivi
esercizioScrivi.o: esercizioScrivi.c
	$(CC) $(CFLAGS) -c esercizioScrivi.c

# file eseguibile leggi
esLeggi: esercizioLeggi.o
	$(CC) $(CFLAGS) -o esercizioLeggi esercizioLeggi.o

# file oggetto leggi
esercizioLeggi.o: esercizioLeggi.c
	$(CC) $(CFLAGS) -c esercizioLeggi.c

clean:
	rm *.o

uninstall:
	rm esercizioLeggi
	rm esercizioScrivi