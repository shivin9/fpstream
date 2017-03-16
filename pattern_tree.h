#ifndef PATTERNTREE
#define PATTERNTREE

#include "sftree.h"

patterntree create_pattern_tree();
pattern_node create_new_pattern_node(data_type);

void create_and_insert_new_child(pattern_node, data);

pattern_node insert_itemset_helper(pattern_node, data, int, float);
patterntree insert_itemset(patterntree, data, int, float);

void update_tilted_tw_table(pattern_node, int, float);
tilted_tw_table insert_batch(tilted_tw_table, int, int, float);
tilted_tw_table create_new_tilted_tw_table(int, int, float);

sfnode dfs(pattern_node);
sftree get_sftree(patterntree);

void delete_tail_freq_table(tilted_tw_table);
void tail_prune(pattern_node);

void print_node(pattern_node);
void print_tree(pattern_node);

#endif //PATTERNTREE