CFLAGS=-Wall -std=c99 -g -O
CC=gcc

# eseguibili da creare
EXECS=primiP primiPsig primiPsigSleep segnaliRT

all: $(EXECS) clean

primiP: primiP.o xerrors.o
	$(CC) $(CFLAGS) -o primiP primiP.o xerrors.o

primiP.o: primiP.c xerrors.c
	$(CC) $(CFLAGS) -c primiP.c -o primiP.o

primiPsig: primiPsig.o xerrors.o
	$(CC) $(CFLAGS) -o primiPsig primiPsig.o xerrors.o

primiPsig.o: primiPsig.c xerrors.c
	$(CC) $(CFLAGS) -c primiPsig.c -o primiPsig.o

primiPsigSleep: primiPsigSleep.o xerrors.o
	$(CC) $(CFLAGS) -o primiPsigSleep primiPsigSleep.o xerrors.o

primiPsigSleep.o: primiPsigSleep.c xerrors.c
	$(CC) $(CFLAGS) -c primiPsigSleep.c -o primiPsigSleep.o

segnaliRT: segnaliRT.o xerrors.o
	$(CC) $(CFLAGS) -o segnaliRT segnaliRT.o xerrors.o

segnaliRT.o: segnaliRT.c xerrors.c
	$(CC) $(CFLAGS) -c segnaliRT.c -o segnaliRT.o

clean:
	rm *.o