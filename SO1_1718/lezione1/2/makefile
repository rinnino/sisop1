# makefile che utilizza regole implicite  e variabili
#
# In questo esempio sfruttiamo la presenza della regola implicita
# che per ottenere n.o da n.c esegue il comando  
#   $(CC) -c $(CFLAGS) -o n.o n.c

# definizione variabili
CFLAGS=-Wall -std=c99 -g -O
CC=gcc

all: esercizio2 clean

# l'esercizio e composto da 2 programmi
esercizio2: es2Scrivi es2Leggi

# file eseguibile scrivi
es2Scrivi: esercizio2scrivi.o
	$(CC) $(CFLAGS) -o esercizio2scrivi esercizio2scrivi.o

# file oggetto scrivi
esercizio2scrivi.o: esercizio2scrivi.c
	$(CC) $(CFLAGS) -c esercizio2scrivi.c

# file eseguibile leggi
es2Leggi: esercizio2leggi.o
	$(CC) $(CFLAGS) -o esercizio2leggi esercizio2leggi.o

# file oggetto leggi
esercizio2leggi.o: esercizio2leggi.c
	$(CC) $(CFLAGS) -c esercizio2leggi.c

clean:
	rm *.o

uninstall:
	rm esercizio2leggi
	rm esercizio2scrivi