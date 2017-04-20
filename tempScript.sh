#R Tree now
#cd grid/RTree/

# KNN for R Tree
sh serialProf.sh 2kD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 100
sh serialProf.sh 10kD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 1000
sh serialProf.sh 100kD100T10.data 1.0 0.01 100 0.05 0.1 3 0.01 1000
sh serialProf.sh 100kD500T10.data 1.0 0.01 100 0.05 0.1 3 0.01 1000
sh serialProf.sh 100kD1000T10.data 1.0 0.01 100 0.05 0.1 3 0.01 1000

# <<COMMENT2
# cd ../RTree/
# sh serialProf.sh 3D_spatial_network_transformed_dense_cells_more_than_3000pts_increased_to_10000pts_withNBH_increased output makefile_profiler 10 20 0.05 4 3D_spatial_network_transformed_sparse_cells_less_than_3000pts_after_increasing_densecells_and_nbhs_to_10000 NBHQUERY
# sh serialProf.sh deluciaD32lac_dense_cells_more_than_3000pts_extto10000_along_withNBH_CellSize2 output makefile_profiler 12 24 2 4 deluciaD32lac_sparse_cells_less_than_3000pts_after_increasing_densecells_and_nbhs_to_10000_CELLSIZE2 NBHQUERY
# sh serialProf.sh deluciaD32lac_dense_cells_more_than_3000pts_extto10000_along_withNBH_CellSize3 output makefile_profiler 12 24 2 4 deluciaD32lac_sparse_cells_less_than_3000pts_after_increasing_densecells_and_nbhs_to_10000_CELLSIZE3 NBHQUERY
# sh serialProf.sh 3D_spatial_network_transformed_dense_cells_more_than_3000pts_increased_to_10000pts_withNBH_increased output makefile_profiler 10 20 0.05 4 3D_spatial_network_transformed_sparse_cells_less_than_3000pts_after_increasing_densecells_and_nbhs_to_10000 KNNQUERY
# sh serialProf.sh deluciaD32lac_dense_cells_more_than_3000pts_extto10000_along_withNBH_CellSize2 output makefile_profiler 12 24 2 4 deluciaD32lac_sparse_cells_less_than_3000pts_after_increasing_densecells_and_nbhs_to_10000_CELLSIZE2 KNNQUERY
# sh serialProf.sh deluciaD32lac_dense_cells_more_than_3000pts_extto10000_along_withNBH_CellSize3 output makefile_profiler 12 24 2 4 deluciaD32lac_sparse_cells_less_than_3000pts_after_increasing_densecells_and_nbhs_to_10000_CELLSIZE3 KNNQUERY
# COMMENT2
