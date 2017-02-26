CC=gcc
FLAG=-pg -fopenmp -c -g

exe: tt1.o twin_tree.o driver.o fpstream.o fptree.o pattern_tree.o
	$(CC) -o exe -pg -fopenmp driver.o fpstream.o fptree.o pattern_tree.o def.h -lm

driver.o: driver.c
	$(CC) $(FLAG) driver.c -lm

tt1.o: tt1.c
	$(CC) $(FLAG) tt1.c -lm

twin_tree.o: twin_tree.c
	$(CC) $(FLAG) twin_tree.c -lm

fpstream.o: fpstream.c
	$(CC) $(FLAG) fpstream.c -lm

fptree.o: fptree.c
	$(CC) $(FLAG) fptree.c -lm

pattern_tree.o: pattern_tree.c
	$(CC) $(FLAG) pattern_tree.c -lm

clean:
	rm -f *.o *~ *.h.gch output intermediate exe*
