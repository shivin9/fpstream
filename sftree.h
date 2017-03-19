#ifndef sfTREE
#define sfTREE

#include "qstack.h"

sftree sf_create_sftree(data_type dat);
sforest sf_create_sforest();
data sf_create_sorted_dummy();
void sf_create_and_insert_new_child(sfnode, data, int);

void sf_append_buffer(sfnode curr, data d, int tid);
void sf_insert_new_child(sfnode, sfnode, data);
void sf_insert_itemset_helper(sfnode, header_table*, int tid);
void sf_insert_itemset(sforest, data, int tid);

void sf_delete_header_table(header_table*);
void sf_delete_tree_structure(sfnode);
void sf_delete_data_node(data);
void sf_delete_buffer(buffer); /* clear up the buffer*/

int sf_size_of_sforest(sforest forest);
int sf_size_of_tree(sfnode);
buffer sf_pop_buffer(sfnode);

int  sf_no_children(sfnode);
int  sf_no_dataitem(sfnode);

void sf_create_header_table_helper(sfnode, header_table*);
void sf_create_header_table(sftree, int);
void sf_update_header_table(header_table*, data, int);

void sf_convert_helper(sfnode, sftree, double*, int*, int tid, int end);
sftree sf_convert_to_CP(sftree, int);
void sf_sort_data(data head, double*);
void sf_sort_header_table(header_table*, double*);
void sf_empty_buffers(sfnode, header_table, int);

data sf_array_to_datalist(int*, int);
void sf_free_data_node(data);

sfnode sf_dfs(sfnode, data_type);
sftree sf_create_conditional_sf_tree(sftree, data_type, double, int);
void sf_mine_frequent_itemsets_helper(sfnode, int*, int end, int pattern);
void sf_mine_frequent_itemsets(sforest, int pattern);

void sf_print_node(sfnode);
void sf_print_tree(sfnode);
void sf_print_sforest(sforest forest);
void sf_print_header_table(header_table*);
void sf_print_data_node(data d);
void sf_print_buffer(sfnode);

data sf_reverse_data(data head);
void sf_prune(sftree, data_type);
void sf_update_ancestor(sfnode);

#endif //sfTREE