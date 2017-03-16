#ifndef SFSTREAM_H
#define SFSTREAM_H

#include "pattern_tree.h"


void print_itemset(data d);
data create_sorted_dummy();
void delete_itemset(data d);
void process_batch(patterntree tree, int batch_num);
<<<<<<< Updated upstream
void fpstream();
=======
void sfstream();
>>>>>>> Stashed changes

#endif