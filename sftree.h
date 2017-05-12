#ifndef sfTREE
#define sfTREE

#include "qstack.h"

/* CREATION FUNCTIONS*/
sftree sf_create_sftree(data_type dat);
sforest sf_create_sforest();
data sf_create_sorted_dummy(int start);
void sf_create_and_insert_new_child(sfnode, data, int);
void sf_create_update_header_node(header_table*, data_type d, int root_data, int tid);

/* INSERTING FUNCTIONS*/
void sf_append_buffer(sfnode curr, data d, double freq, int tid);
void sf_insert_new_child(sfnode, sfnode, int);
void sf_insert_itemset_helper(sfnode, int root_data, int tid);
void sf_fp_insert(sfnode current_node, header_table* htable, data d, double, int tid);
void sf_insert_itemset(sforest, data, int tid);

/* DELETION FUNCTIONS*/
void sf_delete_header_table(header_table*);
void sf_delete_tree_structure(sfnode);
void sf_delete_tree_structure1(sfnode);
void sf_delete_data_node(data);
void sf_delete_buffer(buffer); /* clear up the buffer*/
void sf_delete_buffer_table(bufferTable*);
void sf_delete_sftree(sftree tree);

/* AUXILLARY FUNCTIONS*/
double sf_size_of_sforest(sforest forest);
double sf_size_of_tree(sfnode);
buffer sf_pop_buffer(sfnode, int bucket, int tid);
int  sf_no_children(sfnode);
int  sf_no_dataitem(sfnode);
int sf_get_height(sfnode node);
long unsigned sf_no_of_nodes(sfnode curr);

/* HEADER TABLE FUNCTIONS*/
void sf_create_header_table_helper(sfnode, header_table*);
void sf_create_header_table(sftree, int);
void sf_update_header_table(sftree, int tid);

/* CP-TREE FUNCTIONS*/
void sf_convert_helper(sfnode, sftree, double*, int*, int tid, int end);
sftree sf_convert_to_CP(sftree, int);
void sf_sort_data(data head, double*);
void sf_sort_header_table(header_table*, double*);
void sf_empty_buffers(sforest, int);

/* MISCELLANEOUS FUNCTIONS*/
data sf_array_to_datalist(int*, int);
void sf_free_data_node(data);

/* MINING FUNCTIONS*/
sfnode sf_dfs(sfnode, header_table*, data_type);
sftree sf_create_conditional_sf_tree(sftree, data_type, double, int);
int sf_mine_frequent_itemsets_helper(sfnode, int*, int end, int tid, int pattern);
int sf_mine_frequent_itemsets(sforest, int tid, int pattern);
void sf_fp_mine_frequent_itemsets(sftree tree, data sorted, data till_now, bufferTable collected, int tid, double minsup);
int sf_print_patterns_to_file(int* collected, buffer buff, double cnt, int end, int pattern);

/* PRINTING FUNCTIONS*/
void sf_print_node(sfnode);
void sf_print_tree(sfnode);
void sf_print_sforest(sforest forest);
void sf_print_sforest_lvl(sforest forest);
void sf_print_header_table(header_table*);
void sf_print_data_node(data d);
void sf_print_buffer(buffer);
void sf_print_buffer_table(bufferTable*);

/* PRUNING FUNCTIONS*/
data sf_reverse_data(data head);
int sf_fp_prune(header_table*, int idx, int tid);
void sf_fp_merge1(sfnode parent, sfnode child, int tid);
void sf_prune_buffer(sfnode, int);
void sf_prune_buffer1(sfnode, int);
void sf_prune_helper(sfnode, int root_data, int tid);
void sf_prune(sforest, int);
void sf_update_ancestor(sfnode);

#endif //sfTREE