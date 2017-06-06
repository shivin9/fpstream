CC=gcc
FLAG=-c -Ofast

exe: driver.o fptree.o atstream.o
	$(CC) -Ofast -o exe -pg -fopenmp driver.o fptree.o def.h -lm

driver.o: driver.c
	$(CC) $(FLAG) driver.c -lm

atstream.o: atstream.c
	$(CC) $(FLAG) atstream.c -lm

#twin_tree.o: twin_tree.c
#	$(CC) $(FLAG) twin_tree.c -lm

#fpstream.o: fpstream.c
#	$(CC) $(FLAG) fpstream.c -lm

fptree.o: fptree.c
	$(CC) $(FLAG) fptree.c -lm

#pattern_tree.o: pattern_tree.c
#	$(CC) $(FLAG) pattern_tree.c -lm

clean:
	rm -f exe *.o *~ *.h.gch output intermediate
