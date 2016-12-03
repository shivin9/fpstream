#ifndef FPTREE
#define FPTREE

#include "def.h"

fptree fp_create_fptree();

void fp_create_and_insert_new_child(fpnode, data);
void fp_insert_new_child(fpnode, fpnode, data);
fpnode fp_insert_itemset_helper(fpnode, data, int len, int depth);
fptree fp_insert_itemset(fptree, data, int);
void fp_delete_tree_structure(fpnode);
void fp_delete_data_node(data);
int fp_size_of_tree(fpnode);

int  fp_no_children(fpnode);
int  fp_no_dataitem(fpnode);

void fp_create_header_table_helper(fpnode, header_table*);
void fp_create_header_table(fptree);

void fp_convert_helper(fpnode curr, fptree cptree, int* arr, int* collected, int end);
fptree fp_convert_to_CP(fptree tree);
void fp_sort_data(data head, int* arr);
void fp_sort_header_table(header_table htable, int* table);
void fp_empty_buffers(fpnode curr);
data fp_array_to_datalist(int* arr, int end);
void fp_free_data_node(data d);

fpnode fp_dfs(fpnode, data_type);
fptree fp_create_conditional_fp_tree(fptree, data_type, int);
void fp_mine_frequent_itemsets(fptree, data, data, int);

void fp_print_node(fpnode);
void fp_print_tree(fpnode);
void fp_print_header_table(header_table);
void fp_print_data_node(data d);

data fp_reverse_data(data head);

#endif //FPTREE
