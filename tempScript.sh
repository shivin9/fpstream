#R Tree now
#cd grid/RTree/

# KNN for R Tree
# <dataset> <decay> <eps> <Dict_Size> <m> <M> <TreeLvl> <Sup_cnt> <Batch_Size>

# sh serialProf.sh 1MD100T5.data  1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T5.data  1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD1000T5.data  1.0 0.01 1000 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD1000T10.data 1.0 0.01 1000 0.05 0.1 4 0.01 10000 0.10

sh serialProf.sh 1MD100T15.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD500T15.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD1000T15.data 1.0 0.01 1000 0.05 0.1 4 0.01 10000 0.10

# echo 'studying effect of EPS'
# sh serialProf.sh 1MD100T10.data 1.0 0.005 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.02 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.04 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.06 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.08 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.10 100 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD500T10.data 1.0 0.005 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.02 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.04 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.06 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.08 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.10 500 0.05 0.1 4 0.01 10000 0.10

# echo 'studying the effect of LEVEL'
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 2 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 5 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 6 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 7 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 8 0.01 10000 0.10

# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 2 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 3 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 5 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 6 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 7 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 8 0.01 10000 0.10

# echo 'studying the effect of carry down'
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.20
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.40
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.80
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.20
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.40
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.80
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10

echo 'studying the effect of decay parameter'
sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD100T10.data 0.99995 0.01 100 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD100T10.data 0.99990 0.01 100 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD100T10.data 0.99980 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10

sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD500T10.data 0.99995 0.01 500 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD500T10.data 0.99990 0.01 500 0.05 0.1 4 0.01 10000 0.10
sh serialProf.sh 1MD500T10.data 0.99980 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10

echo 'studying the effect of carry down'
sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 1000 0.10
sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 20000 0.10
sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 5000 0.10
sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 20000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10.data 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10

sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 1000 0.10
sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 2000 0.10
sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 5000 0.10
sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 20000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10.data 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10

