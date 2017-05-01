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
#data_folder=
data_folder=./data/
data_set=$1
Decay=$2
Epsilon=$3
Dict=$4
m=$5
M=$6
Lvl=$7
Sup=$8
Batch=$9
Carry=$10

echo "=========================================="
echo "Following code was profiled on : `date`"
echo "Script for Profiling serial Code"
echo "\nFiles given->\n\t$1"
echo "\nDECAY->$2"
echo "EPS->$3"
echo "DICT->$4"
echo "LVL->$7"
echo "SUP->$8"
echo "BATCH->$9"
echo "CARRY->$10"

echo "=========================================="
echo
# echo "\tCompiling Code..."
# make clean && make -j4
# if [ $? -eq 1 ] ;	then
# 	echo "\tError occured during compilation\nTerminating script\n"
# 	exit 1
# else
# 	echo "\n\tCompilation done successfully.\n"
# fi
#executable file name
exename=bltree
temp=./temp/
tests=./tests
# rm -rf $temp
# mkdir $temp

echo "\t\tRunning Code..."

#sonal di, execute your program here
#$1 and $2 are the command line arguments 
#$1 is input file and $2 is output file to which the program output is to be written


#valgrind --tool=memcheck --leak-check=yes ./output $data_folder$1 output_$1\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt $m $M $Epsilon $MINPOINTS $UNDEFINED $k $data_folder$8 $QT
touch temp1 temp2

(./$exename $data_folder$1 ./temp/Res-[$1\_m=$m\_M=$M\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry].res -m$m -M$M -e$Epsilon -c$Carry -L$Lvl -B$Batch -D$Dict -p2 -d$Decay -s$Sup>temp1) & (./calc_mem.sh > temp2)
cat temp2 >> temp1
gprof ./$exename gmon.out >> temp1
# cat tmp
# python verify.py 

if [ $? -ne 0 ] ;	then
	echo "\tError occured. Terminating script...\n"
	exit 1
fi

echo "Exeuction Done on : `date`"
kill -9 $(pgrep bltree)
#cd $temp
#cd ..
# touch $tempProfiling_result_dataset-$1\_m=$m\_M=$M\_EPS=$Epsilon\_Lvl=$Lvl\_Dict=$Dict\_Decay=$Decay\_Sup=$Sup].prof
cat temp1 > ./temp/ProfRes-[$1\_m=$m\_M=$M\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry].prof
# make clean
rm temp1 temp2 gmon.out 1
#rm -rf $temp
