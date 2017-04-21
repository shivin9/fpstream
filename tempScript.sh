#R Tree now
#cd grid/RTree/

# KNN for R Tree
# <dataset> <decay> <eps> <Dict_Size> <m> <M> <TreeLvl> <Sup_cnt> <Batch_Size>
# sh serialProf.sh 2kD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 100
# sh serialProf.sh 10kD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 1000
# sh serialProf.sh 100kD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 1000
# sh serialProf.sh 100kD500T10.data 1.0 0.01 500 0.05 0.1 3 0.01 1000
# sh serialProf.sh 100kD1000T10.data 1.0 0.01 1000 0.05 0.1 3 0.01 1000
# # increasing the level
# sh serialProf.sh 100kD1000T10.data 1.0 0.01 1000 0.05 0.1 4 0.01 1000
# # increasing the level with less EPS but more pruning
# sh serialProf.sh 500kD500T10.data 0.9995 0.005 500 0.05 0.1 4 0.01 10000
# # changing the pruning interval
# sh serialProf.sh 500kD500T10.data 1.0 0.005 500 0.05 0.1 3 0.01 10000

sh serialProf.sh 1MD1000T15.data 1.0 0.01 1000 0.05 0.1 3 0.01 10000
# introducing decay factor
sh serialProf.sh 1MD1000T15.data 0.9995 0.01 1000 0.05 0.1 3 0.01 10000
# increasing avglen(transaction)
sh serialProf.sh 100kD1000T15.data 0.9995 0.01 1000 0.05 0.1 3 0.01 1000
