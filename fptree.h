#ifndef FPTREE
#define FPTREE

#include "def.h"

fptree fp_create_fptree();
data fp_create_sorted_dummy();

void fp_create_and_insert_new_child(fpnode, data, int);
void fp_insert_new_child(fpnode, fpnode, data);
fpnode fp_insert_itemset_helper(fpnode, header_table, data, int tid, int put_in_buffer);
fptree fp_insert_itemset(fptree, data, int, int);
void fp_delete_tree_structure(fpnode);
void fp_delete_data_node(data);
int fp_size_of_tree(fpnode);

int  fp_no_children(fpnode);
int  fp_no_dataitem(fpnode);

void fp_create_header_table_helper(fpnode, header_table*);
void fp_create_header_table(fptree, int);
void fp_update_header_table(header_table, data, int);

void fp_convert_helper(fpnode, fptree, double*, int*, int tid, int end);
fptree fp_convert_to_CP(fptree, int);
void fp_sort_data(data head, double*);
void fp_sort_header_table(header_table, double*);
void fp_empty_buffers(fpnode, header_table, int);
data fp_array_to_datalist(int*, int);
void fp_free_data_node(data);

fpnode fp_dfs(fpnode, data_type);
fptree fp_create_conditional_fp_tree(fptree, data_type, double, int);
void fp_mine_frequent_itemsets(fptree, data, data, int tid, int pattern);

void fp_print_node(fpnode);
void fp_print_tree(fpnode);
void fp_print_header_table(header_table);
void fp_print_data_node(data d);

data fp_reverse_data(data head);

void fp_prune_infrequent_I_patterns(header_table, data_type, int);
void fp_prune_infrequent_II_patterns(header_table, data_type, int);
void fp_prune_obsolete_II_patterns(header_table, data_type, int);
void fp_prune_obsolete_I_patterns(header_table, data_type, int);
void fp_prune(fptree, data_type);
void fp_update_ancestor(fpnode);
void fp_merge(fpnode, fpnode, header_table);
void fp_merge1(fpnode, fpnode, int);

int fp_ineq7(header_table, int);


#endif //FPTREE
