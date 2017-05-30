#ifndef STRUCT_DEF
#define STRUCT_DEF

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <limits.h>
#include <omp.h>
#include <math.h>

typedef struct timeval timeval;

// GLOBAL VARIABLES
#ifndef GLOBAL_VARS
#define GLOBAL_VARS
extern int DICT_SIZE;
extern double DECAY;
extern long int N;
extern double EPS;
extern double THETA;
extern double CARRY;
extern int HSIZE;
extern double SUP;
extern int BATCH;
extern double MINSUP_SEMIFREQ;
extern double MINSUP_FREQ;
extern int LEAVE_AS_BUFFER;
extern char OUT_FILE[100];
extern int LEAVE_LVL;
extern int BUFFER_SIZE;
extern double GAMMA;
extern double RATE_PARAMETER;
extern unsigned int MAX_BUFFER_SIZE[10];
extern unsigned int MIN_BUFFER_SIZE[10];
extern unsigned int AVG_BUFFER_SIZE[10];
extern unsigned int RED_BUFFER_SIZE[10];
extern unsigned int CNT_BUFFER_SIZE[10];
extern timeval origin;
extern timeval global_timer;
#endif

#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)
#define last_index(x) ((DICT_SIZE) - (x) + (1))
#define index(i,len) ((i) - (len)) /* when the arrays of nodes are of different sizes,
                                      200th item it found at index 100 in the node of item 100*/
#define first(d) (((d)[(0)]) + (2))
#define last(d) (((d)[(0)]) + ((d)[(1)]) + (1))

// -1: something in between
// 0: ready
// 1: converted to CP and ready for pruning and reinitialization
static int T1;
static int T2;
//static int item_ready;
static int batch_ready;

////////////////////////////////////////////////////////////////////////////////
typedef int data_type;    //the data type of individual items in the transaction
typedef int* data; // data[0] has start; data[1] has length

struct buffer_node
{
    data itemset;
    double freq;
    int ftid;
    double ltid;
    struct buffer_node* next;
    // struct buffer_node* prev;
};

typedef struct buffer_node* buffer;

typedef struct buffer_table
{
    buffer bufferhead;
    buffer buffertail;
    double freq;
    int ftid;
    double ltid;
    char collision;
}buffer_table;

typedef buffer_table* bufferTable;
//////////////////////////////////////////////////////////////////////////////

typedef struct sf_node* sfnode; // BL-tree node
typedef struct fp_node* fpnode; // FP-Tree node
typedef struct fptree_node* fptree; // tree which has root node and header table
typedef struct sf_node** sforest; //array of trees for forest
typedef struct header_table_node* header_table; // header table is implemeted as an array (lazy allocation) using this node of this struct.

struct sf_node
{
    sfnode* children; // children in case of BL-tree
    bufferTable* hbuffer; // the hashed buffers
    int bufferSize; // number of nodes in the DLL
    double ltid; // latest updated/seen time stamp of the node. This is used for intermittent pruning.
    int ftid; // first seen tid of the node
    int last; // last timestamp when transaction was appended
    int last_pruned;
    double freq; //count of transaction or item, depending on whether it is used in FP-tree or BL-tree. This is also used to prune along with LTID.
    data_type data_item; // integer data item.
    struct sf_node* parent; // parent pointer in both BL.
    fptree fptree; // contains header table and root node
};

struct fp_node
{
    fpnode child; // first child in case of FP-tree (LL)
    fpnode next; // next pointer of child linked list
    double ltid; // latest updated/seen time stamp of the node. This is used for intermittent pruning.
    int ftid; // first seen tid of the node
    double freq; //count of transaction or item, depending on whether it is used in FP-tree or BL-tree. This is also used to prune along with LTID.
    data_type data_item; // integer data item.
    header_table hnode; // pointer to the header table node in FP-tree
    struct fp_node* next_similar; //pointer to next similar node in FP-tree (DLL originating from Header table; used to make conditional pattern trees)
    struct fp_node* prev_similar; //pointer to prev similar node in FP-tree
    struct fp_node* parent; // parent pointer in both BL as well as FP
    double touched; // used to generated conditional pattern tree
};

struct header_table_node
{
    data_type data_item;
    fpnode first;
    double cnt;
    int ftid;
    double ltid;
};


struct fptree_node
{
    fpnode root;
    header_table* head_table;
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
