#ifndef PATTERNTREE
#define PATTERNTREE

#include "sftree.h"

patterntree create_pattern_tree();
pattern_node create_new_pattern_node(data_type);
pdata create_sorted_dummy();

void create_and_insert_new_child(pattern_node, pdata);

pattern_node insert_itemset_helper(pattern_node, pdata, int, float);
patterntree insert_itemset(patterntree, pdata, int, float);

void update_tilted_tw_table(pattern_node, int, float);
tilted_tw_table insert_batch(tilted_tw_table, int, int, float);
tilted_tw_table create_new_tilted_tw_table(int, int, float);

pfpnode dfs(pattern_node);
pfptree get_fptree(patterntree);

void delete_tail_freq_table(tilted_tw_table);
void tail_prune(pattern_node);

void print_node(pattern_node);
void print_tree(pattern_node);


//////////////////////////////////////////////////////////////////////////////
///////////////////////////// Pattern FP-Tree ////////////////////////////////
//////////////////////////////// Functions ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void fp_create_header_table(pfptree);
void fp_create_header_table_helper(pfpnode, pheader_table*);
void fp_insert_new_child(pfpnode, pfpnode, pdata);
pfptree fp_create_fptree();

pfpnode fp_dfs(pfpnode node, data_type highest_priority_data_item);
pfptree fp_create_conditional_fp_tree(pfptree tree, data_type data_item, int minsup);
void fp_mine_frequent_itemsets(pfptree tree, pdata sorted, pdata till_now, int tid, int pattern);


void fp_delete_data_node(pdata);
void fp_delete_tree_structure(pfpnode);
void fp_delete_header_table(pheader_table);
void fp_delete_fptree(pfptree);

#endif //PATTERNTREE