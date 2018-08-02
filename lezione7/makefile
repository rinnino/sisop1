CFLAGS=-Wall -std=c99 -g -O
CC=gcc

# eseguibili da creare
EXECS=funzfunz sortdemo t3

all: $(EXECS) clean

funzfunz: funzfunz.o xerrors.o
	$(CC) $(CFLAGS) -o funzfunz funzfunz.o xerrors.o

funzfunz.o: funzfunz.c xerrors.c
	$(CC) $(CFLAGS) -c funzfunz.c -o funzfunz.o

sortdemo: sortdemo.o xerrors.o
	$(CC) $(CFLAGS) -o sortdemo sortdemo.o xerrors.o

sortdemo.o: sortdemo.c xerrors.c
	$(CC) $(CFLAGS) -c sortdemo.c -o sortdemo.o

t3: t3.o xerrors.o
	$(CC) $(CFLAGS) -o t3 t3.o xerrors.o -lpthread

t3.o: t3.c xerrors.c
	$(CC) $(CFLAGS) -c t3.c -o t3.o -lpthread

clean:
	rm *.o