CFLAGS=-Wall -std=c99 -g -O
CC=gcc

# eseguibili da creare
EXECS=es4

all: $(EXECS) clean

es4: es4.o xerrors.o
	$(CC) $(CFLAGS) -o es4 es4.o xerrors.o

es4.o: es4.c xerrors.c
	$(CC) $(CFLAGS) -c es4.c -o es4.o

clean:
	rm *.o