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
m=$5
M=$6
Lvl=$7
Sup=$8
Batch=$9
Carry=$10

echo "=========================================="
echo "Following code was profiled on : `date`"
echo "Script for Profiling serial Code using Vampir"
echo "\nFiles given->\n\t$1\n\t$11"
echo "=========================================="
echo
echo "\tCompiling Code..."
make -f $11 clean && make -f $11
if [ $? -eq 1 ] ;	then
	echo "\tError occured during compilation\nTerminating script\n"
	exit 1
else
	echo "\n\tCompilation done successfully.\n"
fi
#executable file name
exename=bltree
temp=./temp
rm -rf $temp
mkdir $temp

#export VT_ON=no
#export VT_UNIFY=yes
#export VT_MAX_FLUSHES=10
#export VT_BUFFER_SIZE=256M
# export VT_MODE=STAT
# export VT_PFORM_GDIR=$temp
# export VT_PFORM_LDIR=$temp
# export VT_FILE_PREFIX=$1
#nm output>output.nm
#export VT_GNU_NMFILE=output.nm
echo "\t\tRunning Code..."

#sonal di, execute your program here
#$1 and $2 are the command line arguments 
#$1 is input file and $2 is output file to which the program output is to be written

#valgrind --tool=memcheck --leak-check=yes ./output $path1$1 output_$1\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt $GMINENTRIES $GMAXENTRIES $GAUXMINENTRIES $GAUXMAXENTRIES $RMINENTRIES $RMAXENTRIES $CELLSIZE $EPSILON $MINPOINTS $UNDEFINED $k $path1$13 $QT

#valgrind --tool=memcheck --leak-check=yes --leak-check=full --show-reachable=yes ./output $path1$1 output_$1\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt $GMINENTRIES $GMAXENTRIES $GAUXMINENTRIES $GAUXMAXENTRIES $RMINENTRIES $RMAXENTRIES $CELLSIZE $EPSILON $MINPOINTS $UNDEFINED

./$exename $data_folder$1 $temp/Res-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry].res -m$m -M$M -e$Epsilon -c$Carry -L$Lvl -B$Batch -D$Dict -p2 -d$Decay -s$Sup
#gprof -z output > gprof_dataset-$2\_m=$m\_M=$M\_EPS=$Epsilon\_MINPOINTS=$MINPOINTS.txt 

if [ $? -ne 0 ] ;	then
	echo "\tError occured. Terminating script...\n"
	exit 1
fi

echo "Exeuction Done on : `date`"
#cd $temp
# vtunify --stats $temp/$1.prof
#cd ..
#cp $temp/$1_$2.prof.txt Profiling_result[dataset-$1\_Gm=$GMINENTRIES\_GM=$GMAXENTRIES\_GAm=$GAUXMINENTRIES\_GAM=$GAUXMAXENTRIES\_Rm=$RMINENTRIES\_RM=$RMAXENTRIES\_EPS=$EPSILON\_MINPOINTS=$MINPOINTS\_CELLSIZE=$CELLSIZE].txt
# cp $temp/$1_$2.prof.txt Profiling_result[dataset-$1\_$QT\_Gm=$GMINENTRIES\_GM=$GMAXENTRIES\_Rm=$RMINENTRIES\_RM=$RMAXENTRIES\_CELLSIZE=$CELLSIZE\_K=$k].txt

cat $temp/$1.prof > $temp/ProfRes-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry].prof
# cat temp1 > ./temp/ProfRes-[$1\_EPS=$Epsilon\_L=$Lvl\_D=$Dict\_d=$Decay\_S=$Sup\_B=$Batch\_c=$Carry].prof

#rm -rf $temp

echo "Unification completed on : `date`"
