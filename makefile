CC=gcc
PROF=-g -pg
OPT=-Ofast
FLAG=$(PROF)


bltree:	qstack.o sftree.o  pattern_tree.o  $(driver).o
	$(CC) $(FLAG) -o bltree sftree.o qstack.o pattern_tree.o $(driver).o def.h -lm

# $(driver): $(driver).o sftree.o qstack.o
# 	$(CC) $(FLAG) -o $(driver) pattern_tree.o sftree.o qstack.o $(driver).o pattern_tree.h -lm

$(driver).o: $(driver).c
	$(CC) -c $(FLAG) $(driver).c -lm

sftree.o: sftree.c
	$(CC) -c $(FLAG) sftree.c -lm

qstack.o: qstack.c
	$(CC) -c $(FLAG) qstack.c -lm

pattern_tree.o: pattern_tree.c
	$(CC) -c $(FLAG) pattern_tree.c -lm

clean:
	rm -f bltree *.o *~ *.h.gch output intermediate
