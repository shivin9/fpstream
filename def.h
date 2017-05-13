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


typedef struct sf_node* sfnode;
typedef struct sftree_node* sftree;
typedef struct sftree_node** sforest;
typedef struct header_table_node* header_table;

struct sf_node
{
    sfnode* children;
    sfnode child;
    sfnode next;
    buffer bufferhead;
    buffer buffertail;
    int bufferSize;
    int ltid; // latest updated/seen time stamp
    int ftid; // first seen tid
    double freq;
    int data_item;
    header_table hnode;
    struct sf_node* next_similar;
    struct sf_node* prev_similar;
    struct sf_node* parent;
    sftree fptree;
    double touched;
};

struct header_table_node
{
    data_type data_item;
    sfnode first;
    double cnt;
    int ftid;
    int ltid;
};


struct sftree_node
{
    sfnode root;
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
