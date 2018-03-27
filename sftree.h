#ifndef sfTREE
#define sfTREE

#include "qstack.h"

/* CREATION FUNCTIONS*/
fptree sf_create_fptree(data_type dat);
sfnode sf_create_sfnode(data_type dat);
sforest sf_create_sforest();
data sf_create_sorted_dummy(int start);
void sf_create_and_insert_new_child(sfnode, data_type, int);
void sf_create_update_header_node(header_table*, data_type, int root_data, int tid);


/* INSERTING FUNCTIONS*/
void sf_append_buffer(sfnode curr, data d, double freq, int tid);
void sf_insert_new_child(fpnode, fpnode, int);
int sf_insert_itemset_helper(sfnode, int root_data, int tid, double, timeval*);
void sf_fp_insert(fpnode, header_table*, data, double, int);
void sf_insert_itemset(sforest, data, int tid, double, timeval*);
void sf_prefix_inset_itemset(sforest forest, data d, double freq, int tid);
void sf_merge_tree(sforest f1, sforest f2, double sup, int tid, int rank);


/* DELETION FUNCTIONS*/
void sf_delete_header_table(header_table *);
void sf_delete_sftree_structure(sfnode current_node);
void sf_delete_fptree_structure(fpnode current_node);
void sf_delete_fptree(fptree tree);
void sf_delete_data_node(data);
void sf_delete_buffer(buffer); /* clear up the buffer*/
void sf_delete_buffer_table(bufferTable*);


/* AUXILLARY FUNCTIONS*/
double sf_size_of_sforest(sforest forest);
double sf_size_of_tree(sfnode);
buffer sf_pop_buffer(sfnode, int bucket, int tid);
buffer sf_get_buffer(sfnode, int bucket, int tid);
int sf_no_sfchildren(sfnode);
int sf_no_fpchildren(fpnode);
int sf_get_height(sfnode node);
long unsigned sf_no_of_nodes(sfnode curr);
char *concat(const char *s1, const char *s2);
char* sf_get_trans(int rank);
buffer sf_string2buffer(char*);


/* HEADER TABLE FUNCTIONS*/
void sf_create_header_table_helper(fpnode, header_table *);
void sf_create_header_table(fptree, int);


/* CP-TREE FUNCTIONS*/
data sf_sort_data(data head);
void sf_sort_header_table(header_table*, double*);
void sf_empty_buffers(sforest, int, double);
void sf_empty_tree(sfnode, int);


/* MISCELLANEOUS FUNCTIONS*/
data sf_array_to_datalist(int*, int);
void sf_free_data_node(data);


/* MINING FUNCTIONS*/
fpnode sf_fp_dfs(fpnode, header_table*, data_type);
fptree sf_create_conditional_fp_tree(fptree, data_type, double, int);
int sf_mine_frequent_itemsets_helper(sfnode, int*, int end, int tid, int pattern, int rank);
int sf_mine_frequent_itemsets(sforest, int tid, int pattern, int rank);
void sf_fp_mine_frequent_itemsets(fptree tree, data_type sorted, data till_now, bufferTable collected, int tid, double minsup);
int sf_print_patterns_to_file(int* collected, buffer buff, double cnt, int end, int pattern, int tid, int rank);


/* PRINTING FUNCTIONS*/
void sf_print_sfnode(sfnode);
void sf_print_fptree(fpnode);
void sf_print_fpnode(fpnode);
void sf_print_buffer(buffer);
void sf_print_sforest(sforest);
void sf_print_data_node(data d);
void sf_print_header_table(header_table*);
void sf_print_buffer_table(bufferTable*);
void sf_print_buffer_node(struct buffer_node node);


/* PRUNING FUNCTIONS*/
data sf_reverse_data(data head);
int sf_fp_prune(header_table*, int idx, int tid);
void sf_fp_merge(fpnode parent, fpnode child, int tid);
void sf_prune_buffer(sfnode, int);
void sf_prune_buffer1(sfnode, int);
void sf_prune_helper(sfnode, int, int);
void sf_prune(sforest, int);
void sf_update_ancestor(sfnode);

#endif //sfTREE