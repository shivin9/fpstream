<<<<<<< Updated upstream
#ifndef sfTREE
#define sfTREE
=======
#ifndef sftree
#define sftree
>>>>>>> Stashed changes

#include "def.h"

sftree sf_create_sftree();
<<<<<<< Updated upstream
=======
sforest sf_create_sforest();
sfForest sf_create_sforest();
>>>>>>> Stashed changes
data sf_create_sorted_dummy();

void sf_create_and_insert_new_child(sfnode, data, int);
void sf_insert_new_child(sfnode, sfnode, data);
<<<<<<< Updated upstream
sfnode sf_insert_itemset_helper(sfnode, data, int, int);
sftree sf_insert_itemset(sftree, data, int, int);
void sf_delete_tree_structure(sfnode);
void sf_delete_data_node(data);
void sf_delete_header_table(header_table*);
=======
sfnode sf_insert_itemset_helper(sfnode, header_table, data, int tid, int buffer);
sftree sf_insert_itemset(sforest, data, int tid, int buffer);
void sf_delete_tree_structure(sfnode);
void sf_delete_data_node(data);
>>>>>>> Stashed changes
int sf_size_of_tree(sfnode);

int  sf_no_children(sfnode);
int  sf_no_dataitem(sfnode);

<<<<<<< Updated upstream
void sf_create_header_table_helper(sfnode, header_table*);
void sf_create_header_table(sftree, int);
void sf_update_header_table(header_table*, data, int);
=======
void sf_create_header_table(sftree, int);
void sf_create_header_table_helper(sfnode, header_table);
void sf_update_header_table(header_table, data, int);
>>>>>>> Stashed changes

void sf_convert_helper(sfnode, sftree, double*, int*, int tid, int end);
sftree sf_convert_to_CP(sftree, int);
void sf_sort_data(data head, double*);
<<<<<<< Updated upstream
void sf_sort_header_table(header_table*, double*);
void sf_empty_buffers(sfnode);
=======
void sf_sort_header_table(header_table, double*);
void sf_empty_buffers(sfnode, header_table, int);
>>>>>>> Stashed changes
data sf_array_to_datalist(int*, int);
void sf_free_data_node(data);

sfnode sf_dfs(sfnode, data_type);
sftree sf_create_conditional_sf_tree(sftree, data_type, double, int);
<<<<<<< Updated upstream
void sf_mine_frequent_itemsets(sftree, data, data, int tid, int pattern);

void sf_print_node(sfnode);
void sf_print_tree(sfnode);
void sf_print_header_table(header_table*);
void sf_print_data_node(data* d);

data sf_reverse_data(data head);

void sf_prune_infrequent_I_patterns(header_table*, data_type, int);
void sf_prune_infrequent_II_patterns(header_table*, data_type, int);
void sf_prune_obsolete_II_patterns(header_table*, data_type, int);
void sf_prune_obsolete_I_patterns(header_table*, data_type, int);
void sf_prune(sftree, data_type);
void sf_update_ancestor(sfnode);
void sf_merge(sfnode, sfnode, header_table*);
int sf_ineq7(header_table, int);


#endif //sfTREE
=======
int sf_mine_frequent_itemsets(sftree, data, data, buffer* , int tid, int pattern);

void sf_print_node(sfnode);
void sf_print_tree(sfnode);
void sf_print_header_table(header_table);
void sf_print_data_node(data d);

data sf_reverse_data(data head);

void sf_prune(sftree, data_type);
void sf_update_ancestor(sfnode);

int sf_ineq7(header_table, int);


#endif //sftree
>>>>>>> Stashed changes
