#ifndef STRUCT_DEF
#define STRUCT_DEF

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <omp.h>
#include <math.h>

#define MINSUP_SEMIFREQ 0.00  //minimum support for semi-frequent itemsets
#define MINSUP_FREQ 0.01    //minimum support for frequent itemsets
#define SUP_ERROR 50 //max error for sub-frequent itemsets
#define DICT_SIZE 1000 // max. number of items
#define SIZE_LMT 64000 // max. size of tree after which it is pruned
#define BATCH_SIZE 10000
// FLAGS
int leave_as_buffer;
static int curr_tree;

// -1: something in between
// 0: ready
// 1: converted to CP and ready for pruning and reinitialization
static int T1;
static int T2;
//static int item_ready;
static int batch_ready;
extern int CNT;
////////////////////////////////////////////////////////////////////////////////
typedef int data_type;    //the data type of individual items in the transaction

// linked list of data_items ie. an itemset
struct data_node{
    data_type data_item;
    struct data_node* next;
};

typedef struct data_node* data;

struct buffer_node{
    data itemset;
    struct buffer_node* next;
};

typedef struct buffer_node* buffer;

//////////////////////////////////////////////////////////////////////////////


typedef struct fp_node* fpnode;
typedef struct fpnode_list_node* fpnode_list;

struct fp_node{
    fpnode_list children;
    data item_list;
    buffer itembuffer;
    int bufferSize;
    int freq;
    data_type data_item;
    struct fp_node* next_similar;
    struct fp_node* parent;
    int touched;
};


struct fpnode_list_node{
    fpnode tree_node; // pointer to fp-tree node
    fpnode_list next; // pointer to next list node
};


struct header_table_node{
    data_type data_item;
    fpnode first;
    int cnt;
    struct header_table_node* next;
};
typedef struct header_table_node* header_table;


struct fptree_node{
    fpnode root;
    header_table head_table;
};
typedef struct fptree_node* fptree;

//////////////////////////////////////////////////////////////////////////////

struct tilted_tw_table{

    int starting_batch;
    int ending_batch;
    int freq;

    int buffer_empty;
    int buffer_starting_batch;
    int buffer_ending_batch;
    int buffer_freq;

    struct tilted_tw_table* next;
};
typedef struct tilted_tw_table* tilted_tw_table;


//////////////////////////////////////////////////////////////////////////////

typedef struct pattern_node_list_node* pattern_node_list;

struct pattern_node_{
    pattern_node_list children;
    data item_list;
    data_type data_item;
    tilted_tw_table table;
};
typedef struct pattern_node_* pattern_node;


struct pattern_node_list_node{
    pattern_node tree_node;
    pattern_node_list next;
};


struct pattern_tree{
    pattern_node root;
};
typedef struct pattern_tree* patterntree;


#endif //STRUCT_DEF
