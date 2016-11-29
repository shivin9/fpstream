#ifndef FPSTREAM_H
#define FPSTREAM_H

#include "pattern_tree.h"


void print_itemset(data d);
data create_sorted_dummy();
void delete_itemset(data d);
void process_batch(patterntree tree, int batch_num);
void fpstream();

#endif