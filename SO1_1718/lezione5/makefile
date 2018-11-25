CFLAGS=-Wall -std=c99 -g -O
CC=gcc

# eseguibili da creare
EXECS=shmsort segnali segnali2 shmerge shmergeSig

all: $(EXECS) clean

shmsort: shmsort.o xerrors.o
	$(CC) $(CFLAGS) -o shmsort shmsort.o xerrors.o

shmsort.o: shmsort.c xerrors.c
	$(CC) $(CFLAGS) -c shmsort.c -o shmsort.o

segnali: segnali.o xerrors.o
	$(CC) $(CFLAGS) -o segnali segnali.o xerrors.o

segnali.o: segnali.c xerrors.c
	$(CC) $(CFLAGS) -c segnali.c -o segnali.o

segnali2: segnali2.o xerrors.o
	$(CC) $(CFLAGS) -o segnali2 segnali2.o xerrors.o

segnali2.o: segnali2.c xerrors.c
	$(CC) $(CFLAGS) -c segnali2.c -o segnali2.o

shmerge: shmerge.o xerrors.o
	$(CC) $(CFLAGS) -o shmerge shmerge.o xerrors.o

shmerge.o: shmerge.c xerrors.c
	$(CC) $(CFLAGS) -c shmerge.c -o shmerge.o

shmergeSig: shmergeSig.o xerrors.o
	$(CC) $(CFLAGS) -o shmergeSig shmergeSig.o xerrors.o

shmergeSig.o: shmergeSig.c xerrors.c
	$(CC) $(CFLAGS) -c shmergeSig.c -o shmergeSig.o

clean:
	rm *.o