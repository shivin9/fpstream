output: Data.o GList.o RList.o BCell.o PQueue.o RTree.o GridRTree.o Test_GRTree.o Main_Grid.o
	gcc -o output Data.o GList.o RList.o BCell.o PQueue.o RTree.o GridRTree.o Test_GRTree.o Main_Grid.o -lm  

Main_Grid.o: Main_Grid.c
	gcc -c -g Main_Grid.c -lm

Data.o: Data.c
	gcc -c -g Data.c -lm
	
GridRTree.o: GridRTree.c
	gcc -c -g GridRTree.c -lm

GList.o: GList.c
	gcc -c -g GList.c -lm 

RList.o: RList.c
	gcc -c -g RList.c -lm 

BCell.o: BCell.c
	gcc -c -g BCell.c -lm 

RTree.o: RTree.c
	gcc -c -g RTree.c -lm

Test_GRTree.o: Test_GRTree.c
	gcc -c -g Test_GRTree.c -lm
	
PQueue.o: PQueue.c
	gcc -c -g PQueue.c -lm

clean:
	rm *.o
	rm output