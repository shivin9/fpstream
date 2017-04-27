#R Tree now
#cd grid/RTree/

# KNN for R Tree
# <dataset> <decay> <eps> <Dict_Size> <m> <M> <TreeLvl> <Sup_cnt> <Batch_Size>

# DICT_SIZE = 100
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T15.data 1.0 0.01 100 0.05 0.1 4 0.05 10000

# DICT_SIZE = 500
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T15.data 1.0 0.01 500 0.05 0.1 4 0.05 10000

# DICT_SIZE = 1000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 1000 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T15.data 1.0 0.01 1000 0.05 0.1 4 0.05 10000

## studying effect of EPS
sh serialProf.sh 1MD1000T10.data 1.0 0.005 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.02 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.04 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.06 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.08 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.10 500 0.05 0.1 4 0.05 10000

sh serialProf.sh 1MD1000T10.data 1.0 0.005 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.02 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.04 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.06 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.08 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.10 100 0.05 0.1 4 0.05 10000

## studying the effect of LEVEL
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 2 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 3 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 5 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 6 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 7 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 500 0.05 0.1 8 0.05 10000

sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 2 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 3 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 4 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 5 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 6 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 7 0.05 10000
sh serialProf.sh 1MD1000T10.data 1.0 0.01 100 0.05 0.1 8 0.05 10000
