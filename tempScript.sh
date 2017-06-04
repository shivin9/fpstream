# BLTree
# <dataset> <decay> <eps> <Dict_Size> <TreeLvl> <Sup_cnt> <Batch_Size> <Carry>\
# <Theta> <Gamma> <Rate> <MAX_Buffer> <Hash%>

# sh serialProf.sh 1MD100T10 1.0 0.005 100 4 0.01 10000 0.10 0.05 10 100
# sh serialProf.sh 1MD500T10 1.0 0.005 500 4 0.01 10000 0.10 0.05 10 100


# sh serialProf.sh 1MD100T10 1.0 0.005 100 4 0.01 10000 0.10 0.05 20 100
# sh serialProf.sh 1MD500T10 1.0 0.005 500 4 0.01 10000 0.10 0.05 20 100

# sh serialProf.sh 1MD100T10 1.0 0.005 100 4 0.01 10000 0.10 0.05 25 100
# sh serialProf.sh 1MD500T10 1.0 0.005 500 4 0.01 10000 0.10 0.05 25 100

# sh serialProf.sh 1MD1000T5 1.0 0.005 1000 4 0.01 10000 0.10 0.05 25 100
# sh serialProf.sh 1MD1000T10 1.0 0.005 1000 4 0.01 10000 0.10 0.05 25 100

# Results to prove that recall falls with speed
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 5 100 0.1
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 15 100 0.1
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 10 100 0.1
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 20 100 0.1
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 25 100 0.1
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 30 100 0.1
#sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 35 100 0.1
# sh serialProf.sh 1MD1000T10 1.0 0.0025 1000 4 0.003 10000 0.10 0.05 2 40 100 0.1

# BIG Datasets
#make clean
#make driver=driver
#sh serialProf.sh 10MD1000T15 1.0 0.0025 1000 4 0.01 10000 0.10 0.05 3.0 10 200 0.1
#sh serialProf.sh 10MD1000T15 1.0 0.0025 1000 4 0.01 10000 0.10 0.05 3.0 20 200 0.1
#sh serialProf.sh 10MD1000T15 1.0 0.0025 1000 4 0.01 10000 0.10 0.05 3.0 30 200 0.1
#sh serialProf.sh 10MD1000T15 1.0 0.0025 1000 4 0.01 10000 0.10 0.05 3.0 40 200 0.1
