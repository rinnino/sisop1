# makefile che utilizza regole implicite  e variabili
#
# In questo esempio sfruttiamo la presenza della regola implicita
# che per ottenere n.o da n.c esegue il comando  
#   $(CC) -c $(CFLAGS) -o n.o n.c

# definizione variabili
CFLAGS=-Wall -std=c99 -g -O
CC=gcc

all: esercizio1 clean

# l'esercizio e composto da 2 programmi
esercizio1: es1Scrivi es1Leggi

# file eseguibile scrivi
es1Scrivi: esercizio1scrivi.o
	$(CC) $(CFLAGS) -o esercizio1scrivi esercizio1scrivi.o

# file oggetto scrivi
esercizio1scrivi.o: esercizio1scrivi.c
	$(CC) $(CFLAGS) -c esercizio1scrivi.c

# file eseguibile leggi
es1Leggi: esercizio1leggi.o
	$(CC) $(CFLAGS) -o esercizio1leggi esercizio1leggi.o

# file oggetto leggi
esercizio1leggi.o: esercizio1leggi.c
	$(CC) $(CFLAGS) -c esercizio1leggi.c

clean:
	rm *.o

uninstall:
	rm esercizio1leggi
	rm esercizio1scrivi