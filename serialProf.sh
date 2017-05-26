#
#	This script profiles MPI code using VampirTrace
#
#	File dependence on 	:	makefile	(Instrumented according to VampirTrace)
#	Input				:	Dimesion of Input data,1 input file and 1 output file passed as command line arguments
#	Output 				:	Profiling_result[dataset-$2_m=$m_M=$M_EPS=$Epsilon_MINPOINTS=$MINPOINTS].txt
#
#	Author 				:	Saiyedul Islam
#		 					saiyedul.islam@gmail.com
#
#parameters for running code
# path1=../../../Dataset/
# #path1=
# EPSILON=${10}
# MINPOINTS=2000
# UNDEFINED=10000000
# GMINENTRIES=$4	
# GMAXENTRIES=$5
# GAUXMINENTRIES=$6
# GAUXMAXENTRIES=$7
# RMINENTRIES=$8
# RMAXENTRIES=$9
# CELLSIZE=${11}
# k=${12}
# QT=${14}

data_folder=./data/
data_set=$1
Decay=$2
Epsilon=$3
Dict=$4
Lvl=$5
Sup=$6
Batch=$7
Carry=$8
Theta=$9
Rate=$10
Buffer=$11


echo "=========================================="
echo "Following code was profiled on : `date`"
echo "Script for Profiling serial Code"
echo "\nFiles given->\n\t$1"
echo "\nDECAY->$2"
echo "EPS->$3"
echo "DICT->$4"
echo "LVL->$5"
echo "SUP->$6"
echo "BATCH->$7"
echo "CARRY->$8"
echo "Theta->$9"
echo "Rate->$10"
echo "Buffer->$11"

echo "=========================================="

# echo "\tCompiling Code..."
# make -f $11 clean && make -f $11
# if [ $? -eq 1 ] ;	then
# 	echo "\tError occured during compilation\nTerminating script\n"
# 	exit 1
# else
# 	echo "\n\tCompilation done successfully.\n"
# fi
#executable file name
exename=bltree
temp=./temp
tests=./tests

echo "\t\tRunning Code..."

#sonal di, execute your program here
#$1 and $2 are the command line arguments 
#$1 is input file and $2 is output file to which the program output is to be written

#valgrind --tool=memcheck --leak-check=yes ./output $path1$1 output_$1\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt $GMINENTRIES $GMAXENTRIES $GAUXMINENTRIES $GAUXMAXENTRIES $RMINENTRIES $RMAXENTRIES $CELLSIZE $EPSILON $MINPOINTS $UNDEFINED $k $path1$13 $QT

#valgrind --tool=memcheck --leak-check=yes --leak-check=full --show-reachable=yes ./output $path1$1 output_$1\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt $GMINENTRIES $GMAXENTRIES $GAUXMINENTRIES $GAUXMAXENTRIES $RMINENTRIES $RMAXENTRIES $CELLSIZE $EPSILON $MINPOINTS $UNDEFINED
touch temp1

./$exename $data_folder$1 ./temp/Res-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry\_Theta=$Theta\_Rate=$Rate\_Buffer=$Buffer].res -e$Epsilon -c$Carry -L$Lvl -B$Batch -D$Dict -p2 -d$Decay -s$Sup -r$Rate -t$Theta -b$Buffer
#gprof -z output > gprof_dataset-$2\_m=$m\_M=$M\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt 
gprof ./$exename gmon.out >> temp1

if [ $? -ne 0 ] ;	then
	echo "\tError occured. Terminating script...\n"
	exit 1
fi

echo "Exeuction Done on : `date`"
# kill -9 $(pgrep bltree)
#cd $temp
#cd ..
# touch $tempProfiling_result_dataset-$1\_m=$m\_M=$M\_EPS=$Epsilon\_Lvl=$Lvl\_Dict=$Dict\_Decay=$Decay\_Sup=$Sup].prof
cat temp1 > $temp/ProfRes-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry\_Theta=$Theta\_Rate=$Rate\_Buffer=$Buffer].prof
rm temp1 gmon.out
