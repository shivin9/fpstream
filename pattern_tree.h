#ifndef PATTERNTREE
#define PATTERNTREE

#include "fptree.h"

patterntree create_pattern_tree();
pattern_node create_new_pattern_node(data_type);

void create_and_insert_new_child(pattern_node, data);

pattern_node insert_itemset_helper(pattern_node, data, int, int);
patterntree insert_itemset(patterntree, data, int, int);

void update_tilted_tw_table(pattern_node, int, int);
tilted_tw_table insert_batch(tilted_tw_table, int, int, int);
tilted_tw_table create_new_tilted_tw_table(int, int, int);

fpnode dfs(pattern_node);
fptree get_fptree(patterntree);

void delete_tail_freq_table(tilted_tw_table);
void tail_prune(pattern_node);

void print_node(pattern_node);
void print_tree(pattern_node);

#endif //PATTERNTREE