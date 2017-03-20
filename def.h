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
#define last_index(x) ((DICT_SIZE) - (x) + (1))
#define index(i,len) ((i) - (len)) /* when the arrays of nodes are of different sizes,
                                      200th item it found at index 100 in the node of item 100*/

#define DECAY 1.0
#define DICT_SIZE 100 // max. number of items
#define N 5000 //window size
#define EPS 0.0001
#define THETA 0.001
#define HSIZE 10000

#define SUP 18.0
#define BATCH 15

#define MINSUP_SEMIFREQ 10  //minimum support for semi-frequent itemsets
#define MINSUP_FREQ 8    //minimum support for frequent itemsets
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

// linked list of data_items ie. an itemset
struct data_node
{
    data_type data_item;
    struct data_node* next;
};

typedef struct data_node* data;

struct buffer_node
{
    data itemset;
    int tid;
    struct buffer_node* next;
};

typedef struct buffer_node* buffer;

//////////////////////////////////////////////////////////////////////////////


typedef struct sf_node* sfnode;
typedef struct header_table_node* header_table;

struct sf_node{
    sfnode* children;
    data* item_list;
    buffer bufferhead;
    buffer buffertail;
    int bufferSize;
    int tid; // time stamp
    double freq;
    data_type data_item;
    header_table hnode;
    struct sf_node* next_similar;
    struct sf_node* prev_similar;
    struct sf_node* parent;
    double touched;
};


struct header_table_node{
    data_type data_item;
    sfnode first;
    double cnt;
    int tid;
};


struct sftree_node{
    sfnode root;
    header_table* head_table;
};
typedef struct sftree_node* sftree;
typedef struct sftree_node** sforest;

//////////////////////////////////////////////////////////////////////////////
typedef struct snode snode;
typedef snode* slink;
typedef struct QStack QStack;
typedef struct tuple tuple;
typedef struct dictionary dictionary;
typedef struct dictionary* dict;
typedef struct hnode hnode;
typedef hnode* hlink;

// the qstack node which is a part of a doubly linked list
struct snode
{
    sfnode node;
    slink next;
    slink prev;
};

// a QStack ie. a stack and a queue simultaneously implemented using a doubly linked list
struct QStack
{
    slink head;
    slink tail;
    int size;
};

// a node used in the hash table
struct hnode
{
    char *sig;
    hlink next;
};

// the hash table data structure called a dictionary
struct dictionary
{
    hlink table[HSIZE];
    int size;
};

//////////////////////////////////////////////////////////////////////////////

struct tilted_tw_table
{
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
typedef struct pattern_node_list_node* pattern_node_list;

struct pattern_node_
{
    pattern_node_list children;
    data item_list;
    data_type data_item;
    tilted_tw_table table;
};
typedef struct pattern_node_* pattern_node;


struct pattern_node_list_node
{
    pattern_node tree_node;
    pattern_node_list next;
};


struct pattern_tree
{
    pattern_node root;
};
typedef struct pattern_tree* patterntree;


#endif //STRUCT_DEF
