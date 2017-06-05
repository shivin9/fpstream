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
GAMMA=$10
Rate=$11
Buffer=$12
Hash=$13
TimeM=$14

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
echo "THETA->$9"
echo "GAMMA->$10"
echo "RATE->$11"
echo "Buffer->$12"
echo "HSize->$13"
echo "TimeM->$14"
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

#$1 and $2 are the command line arguments 
#$1 is input file and $2 is output file to which the program output is to be written
touch temp1 temp2
input_file=$1\.data
output_file=$temp/Res-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry\_Theta=$Theta\_Rate=$Rate\_Buffer=$Buffer\_Gamma=$GAMMA\_Hash=$HSize\_TimeM=$TimeM].res

./$exename $data_folder$input_file $output_file -e$Epsilon -c$Carry -L$Lvl -T$TimeM -B$Batch -D$Dict -p2 -d$Decay -s$Sup -r$Rate -t$Theta -b$Buffer -g$GAMMA -H$Hash > temp1 & (./calc_mem.sh > temp2)

if [ $? -ne 0 ] ;	then
	echo "\tError occured. Terminating script...\n"
	exit 1
fi
kill -9 $(pgrep bltree)

per_sup=`echo $6*100|bc`
#gprof -z output > gprof_dataset-$2\_m=$m\_M=$M\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt 
# gprof ./$exename gmon.out >> temp1
echo "converting input data file to tab file"
python convert.py $input_file
rm ./tests/res.gnd
tail -100000 ./tests/$1\.tab >tmp && ./fpgrowth -s$per_sup tmp ./tests/res.gnd && rm tmp # send tab file for recall check
python verify.py $output_file res.gnd >> temp1

echo "Exeuction Done on : `date`"
# kill -9 $(pgrep bltree)
#cd $temp
#cd ..
# touch $tempProfiling_result_dataset-$1\_m=$m\_M=$M\_EPS=$Epsilon\_Lvl=$Lvl\_Dict=$Dict\_Decay=$Decay\_Sup=$Sup].prof
cat temp2 >> temp1
cat temp1 >> $temp/ProfRes-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry\_Theta=$Theta\_Rate=$Rate\_Buffer=$Buffer\_TimeM=$TimeM].prof
rm temp1 temp2
