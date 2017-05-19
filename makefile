CC=gcc
FLAG=-c -g -pg

bltree: driver.o sftree.o qstack.o
	$(CC) -o bltree sftree.o qstack.o driver.o def.h -lm

driver.o: driver.c
	$(CC) $(FLAG) driver.c -lm

sftree.o: sftree.c
	$(CC) $(FLAG) sftree.c -lm

qstack.o: qstack.c
	$(CC) $(FLAG) qstack.c -lm

clean:
	rm -f bltree *.o *~ *.h.gch output intermediate
