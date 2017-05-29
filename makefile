CC=gcc
PROF=-g -pg
OPT=-Ofast
FLAG=$(PROF) 

bltree: $(driver).o sftree.o qstack.o
	$(CC) $(FLAG) -o bltree sftree.o qstack.o $(driver).o def.h -lm

$(driver).o: $(driver).c
	$(CC) -c $(FLAG) $(driver).c -lm

sftree.o: sftree.c
	$(CC) -c $(FLAG) sftree.c -lm

qstack.o: qstack.c
	$(CC) -c $(FLAG) qstack.c -lm

clean:
	rm -f bltree *.o *~ *.h.gch output intermediate
