#ifndef STRUCT_DEF
#define STRUCT_DEF

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "./BitArray/bar.h"
#include <malloc.h>
#include <limits.h>
#include <omp.h>
#include <math.h>

// GLOBAL VARIABLES
#ifndef GLOBAL_VARS
#define GLOBAL_VARS
extern int DICT_SIZE;
extern double DECAY;
extern long int N; // current length of stream
extern double EPS;
extern double THETA; // Not used
extern double CARRY; // carry down parameter
extern int HSIZE; // size of the hash table for buffer
extern double SUP; // support count for mining
extern int BATCH; // batch size for intermittent pruning
extern double MINSUP_SEMIFREQ; // not used
extern double MINSUP_FREQ; // not used
extern int LEAVE_AS_BUFFER; // FLAG to tell when to pre-empt. Will be used in anytime simulation.
extern int LEAVE_LVL; // level at which FP-tree exists in BL-tree
extern char OUT_FILE[100];
#endif

#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)
#define last_index(x) ((DICT_SIZE) - (x) + (1))
#define index(i,len) ((i) - (len)) /* when the arrays of nodes are of different sizes,
                                      200th item it found at index 100 in the node of item 100*/
#define bar BIT_ARRAY
////////////////////////////////////////////////////////////////////////////////
typedef bar* data;
typedef int data_type;
typedef bit_index_t bit;

struct buffer_node
{
    data itemset;
    double freq;
    int ftid;
    int ltid;
    struct buffer_node* next;
    struct buffer_node* prev;
};

typedef struct buffer_node* buffer;

//////////////////////////////////////////////////////////////////////////////
// Definitions for the trees in our data structure

typedef struct sf_node* sfnode; // BL-tree node
typedef struct fp_node* fpnode; // FP-Tree node
typedef struct fptree_node* fptree; // tree which has root node and header table
typedef struct sf_node** sforest; //array of trees for forest
typedef struct header_table_node* header_table; // header table is implemeted as an array (lazy allocation) using this node of this struct.


struct sf_node
{
    sfnode* children; // children in case of BL-tree
    buffer bufferhead; // head of buffer (DLL)
    buffer buffertail; // tail of buffer (DLL)
    int bufferSize; // number of nodes in the DLL
    int ltid; // latest updated/seen time stamp of the node. This is used for intermittent pruning.
    int ftid; // first seen tid of the node
    double freq; //count of transaction or item, depending on whether it is used in FP-tree or BL-tree. This is also used to prune along with LTID.
    data_type data_item; // integer data item.
    struct sf_node* parent; // parent pointer in both BL as well as FP
    fptree fptree; // node for FP-tree (whole tree)
};

struct fp_node
{
    fpnode child; // first child in case of FP-tree (LL)
    fpnode next; // next pointer of child linked list
    int ltid; // latest updated/seen time stamp of the node. This is used for intermittent pruning.
    int ftid; // first seen tid of the node
    double freq; //count of transaction or item, depending on whether it is used in FP-tree or BL-tree. This is also used to prune along with LTID.
    data_type data_item; // integer data item.
    header_table hnode; // pointer to the header table in FP-tree
    struct fp_node* next_similar; //pointer to next similar node in FP-tree (DLL originating from Header table; used to make conditional pattern trees)
    struct fp_node* prev_similar; //pointer to prev similar node in FP-tree
    struct fp_node* parent; // parent pointer in both BL as well as FP
    double touched; // used to generated conditional pattern tree
};


struct header_table_node //  header table node
{
    data_type data_item;
    fpnode first; // pointer to first entry in FP-tree
    double cnt; // cumulative count of all similar nodes
    int ftid; // first seen transaction id. Used in pruning FP-trees.
    int ltid; // latest updated transaction id
};


struct fptree_node
{
    fpnode root;
    header_table* head_table; // header table array. Segregate this for FP-tree and not to be used in BL-tree
};

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
    hlink table[10000];
    int size;
};


#endif //STRUCT_DEF
