#ifndef FPSTREAM_H
#define FPSTREAM_H

#include "pattern_tree.h"


void delete_itemset(pdata d);
void process_batch(patterntree tree, int batch_num);
void fpstream();

#endif