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

#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)

#define DECAY 0.99
#define DICT_SIZE 1000 // max. number of items
#define N 1000000 //window size
#define EPS 0.0
#define THETA 0.01
#define BATCH_SIZE 10000

#define SUP 18.0
#define BATCH 15

#define MINSUP_SEMIFREQ N*(THETA-EPS)  //minimum support for semi-frequent itemsets
#define MINSUP_FREQ N*(THETA)    //minimum support for frequent itemsets
#define SUP_ERROR 50.0 //max error for sub-frequent itemsets
#define SIZE_LMT 16192 // max. size of tree after which it is pruned


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

////////////////////////////////////////////////////////////////////////////////
typedef int data_type;    //the data type of individual items in the transaction
typedef struct timeval timeval;
extern timeval origin;
extern timeval global_timer;

// linked list of data_items ie. an itemset
struct data_node{
    data_type data_item;
    struct data_node* next;
};

typedef struct data_node* data;

struct buffer_node{
    data itemset;
    int tid;
    struct buffer_node* next;
};

typedef struct buffer_node* buffer;

//////////////////////////////////////////////////////////////////////////////


typedef struct fp_node* fpnode;
typedef struct fpnode_list_node* fpnode_list;
typedef struct header_table_node* header_table;

struct fp_node{
    fpnode_list children;
    data item_list;
    buffer itembuffer;
    int bufferSize;
    int tid; // time stamp
    double freq;
    data_type data_item;
    header_table hnode;
    struct fp_node* next_similar;
    struct fp_node* prev_similar;
    struct fp_node* parent;
    double touched;
};


struct fpnode_list_node{
    fpnode tree_node; // pointer to fp-tree node
    fpnode_list next; // pointer to next list node
};


struct header_table_node{
    data_type data_item;
    fpnode first;
    double cnt;
    int tid;
    struct header_table_node* next;
};


struct fptree_node{
    fpnode root;
    header_table head_table;
};
typedef struct fptree_node* fptree;

//////////////////////////////////////////////////////////////////////////////

struct tilted_tw_table{

    int starting_batch;
    int ending_batch;
    double freq;

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
