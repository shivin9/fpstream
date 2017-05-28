# BLTree
# <dataset> <decay> <eps> <Dict_Size> <TreeLvl> <Sup_cnt> <Batch_Size> <Carry>\
# <Theta> <Gamma> <Rate> <MAX_Buffer> <HashSize>

# sh serialProf.sh 1MD100T10 1.0 0.005 100 4 0.01 10000 0.10 0.05 10 100
# sh serialProf.sh 1MD500T10 1.0 0.005 500 4 0.01 10000 0.10 0.05 10 100
# sh serialProf.sh 1MD1000T10 1.0 0.005 1000 4 0.01 10000 0.10 0.05 10 100

# sh serialProf.sh 1MD100T10 1.0 0.005 100 4 0.01 10000 0.10 0.05 20 100
# sh serialProf.sh 1MD500T10 1.0 0.005 500 4 0.01 10000 0.10 0.05 20 100
# sh serialProf.sh 1MD1000T10 1.0 0.005 1000 4 0.01 10000 0.10 0.05 20 100

# sh serialProf.sh 1MD100T10 1.0 0.005 100 4 0.01 10000 0.10 0.05 25 100
# sh serialProf.sh 1MD500T10 1.0 0.005 500 4 0.01 10000 0.10 0.05 25 100
# sh serialProf.sh 1MD1000T10 1.0 0.005 1000 4 0.01 10000 0.10 0.05 25 100

# sh serialProf.sh 1MD1000T5 1.0 0.005 1000 4 0.01 10000 0.10 0.05 25 100
# sh serialProf.sh 1MD1000T10 1.0 0.005 1000 4 0.01 10000 0.10 0.05 25 100
# sh serialProf.sh 1MD1000T15 1.0 0.005 1000 4 0.01 10000 0.10 0.05 25 100

# BIG Datasets
# make clean
# make driver=driver_10M
# sh serialProf.sh 10MD500T15 1.0 0.0025 500 4 0.01 10000 0.10 0.05 2.0 20 1000
# sh serialProf.sh 10MD1000T15 1.0 0.0025 1000 4 0.01 10000 0.10 0.05 3.0 20 200

# Normal tests
# sh serialProf.sh 1MD100T5  1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T5  1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD1000T5  1.0 0.01 1000 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 0.1kD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10 makefile_profiler
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD1000T10 1.0 0.01 1000 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD100T15 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T15 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD1000T15 1.0 0.01 1000 0.05 0.1 4 0.01 10000 0.10

# echo "=========================================="
# echo 'studying effect of EPS'
# echo "=========================================="

# sh serialProf.sh 1MD100T10 1.0 0.005 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.02 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.04 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10 1.0 0.06 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.08 100 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD100T10 1.0 0.10 100 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD500T10 1.0 0.005 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.02 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.04 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10 1.0 0.06 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.08 500 0.05 0.1 4 0.01 10000 0.10
# # sh serialProf.sh 1MD500T10 1.0 0.10 500 0.05 0.1 4 0.01 10000 0.10

# echo "=========================================="
# echo 'studying the effect of LEVEL'
# echo "=========================================="

# sh serialProf1.sh 1MD100T10 1.0 500 100 0.05 0.1 2 0.01 10000 0.10
# sh serialProf1.sh 1MD100T10 1.0 500 100 0.05 0.1 3 0.01 10000 0.10
# sh serialProf1.sh 1MD100T10 1.0 500 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf1.sh 1MD100T10 1.0 500 100 0.05 0.1 5 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 6 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 7 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 8 0.01 10000 0.10

# sh serialProf1.sh 1MD500T10 1.0 500 500 0.05 0.1 2 0.01 10000 0.10
# sh serialProf1.sh 1MD500T10 1.0 500 500 0.05 0.1 3 0.01 10000 0.10
# sh serialProf1.sh 1MD500T10 1.0 500 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf1.sh 1MD500T10 1.0 500 500 0.05 0.1 5 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 6 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 7 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 8 0.01 10000 0.10

# echo "=========================================="
# echo 'studying the effect of carry down'
# echo "=========================================="
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.20
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.40
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.80
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.20
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.40
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.80
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10

# echo "=========================================="
# echo 'studying the effect of decay parameter'
# echo "=========================================="

# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 0.99995 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 0.99990 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 0.99980 0.01 100 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 0.99995 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 0.99990 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 0.99980 0.01 500 0.05 0.1 4 0.01 10000 0.10

# echo "=========================================="
# echo 'studying the effect of batch size'
# echo "=========================================="
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 1000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 20000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 5000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 20000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10

# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 1000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 2000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 5000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 20000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10

# git add -A
# git commit --amend --no-edit
# git checkout bltree
# make clean && make
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# mv temp temp_bltree

# git add -A
# git commit --amend --no-edit
# git checkout new_buffer
# make clean && make
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# mv temp temp_new_buffer

# git add -A
# git commit --amend --no-edit
# git checkout bit_vector
# make clean && make
# sh serialProf.sh 1MD100T10 1.0 0.01 100 0.05 0.1 4 0.01 10000 0.10
# sh serialProf.sh 1MD500T10 1.0 0.01 500 0.05 0.1 4 0.01 10000 0.10
# mv temp temp_bit_vector
