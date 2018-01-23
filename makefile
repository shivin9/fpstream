CC=gcc
PROF=-g -pg
OPT=-Ofast
FLAG=$(PROF)


bltree:	mp_bltree.o sfstream.o qstack.o sftree.o pattern_tree.o 
	$(CC) $(FLAG) -o bltree sfstream.o sftree.o qstack.o pattern_tree.o mp_bltree.o def.h -lm -fopenmp

# $(driver): $(driver).o sftree.o qstack.o
# 	$(CC) $(FLAG) -o $(driver) pattern_tree.o sftree.o qstack.o $(driver).o pattern_tree.h -lm

# $(driver).o: $(driver).c
# 	$(CC) -c $(FLAG) -openmp $(driver).c -lm


mp_bltree.o: mp_bltree.c
	$(CC) -c $(FLAG) mp_bltree.c -lm -fopenmp

sftree.o: sftree.c
	$(CC) -c $(FLAG) sftree.c -lm

sfstream.o: sfstream.c
	$(CC) -c $(FLAG) sfstream.c -lm

qstack.o: qstack.c
	$(CC) -c $(FLAG) qstack.c -lm

pattern_tree.o: pattern_tree.c
	$(CC) -c $(FLAG) pattern_tree.c -lm

clean:
	rm -f bltree *.o *~ *.h.gch output intermediate
