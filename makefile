CC=gcc
FLAG=-pg -fopenmp -c -g 

exe: driver.o sftree.o
	$(CC) -o exe -pg -fopenmp sftree.o driver.o def.h -lm

driver.o: driver.c
	$(CC) $(FLAG) driver.c -lm

# atstream.o: atstream.c
# 	$(CC) $(FLAG) atstream.c -lm

#twin_tree.o: twin_tree.c
#	$(CC) $(FLAG) twin_tree.c -lm

#sfstream.o: sfstream.c
#	$(CC) $(FLAG) sfstream.c -lm

sftree.o: sftree.c
	$(CC) $(FLAG) sftree.c -lm

#pattern_tree.o: pattern_tree.c
#	$(CC) $(FLAG) pattern_tree.c -lm

clean:
	rm -f exe *.o *~ *.h.gch output intermediate
