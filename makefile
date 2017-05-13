CC=gcc
BIT_ARR_PATH=./BitArray/
CFLAGS= -g -pg
LIBS=-I$(BIT_ARR_PATH) -L$(BIT_ARR_PATH)

bltree: driver.o sftree.o qstack.o
		$(CC) $(CFLAGS) $(LIBS) -o bltree sftree.c qstack.c driver.c def.h -lm -lbitarr

# driver.o: driver.c #$(BIT_ARR_PATH)libbitarr.a
# 		$(CC) $(CFLAGS) $(LIBS) driver.c -lm -lbitarr

# sftree.o: sftree.c #$(BIT_ARR_PATH)libbitarr.a
# 		$(CC) $(CFLAGS) $(LIBS) sftree.c -lm -lbitarr

# qstack.o: qstack.c #def.h $(BIT_ARR_PATH)libbitarr.a
# 		$(CC) $(CFLAGS) $(LIBS) qstack.c -lm -lbitarr

clean:
		rm -f bltree *.o *~ *.h.gch output intermediate
