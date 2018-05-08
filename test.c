#include "sfstream.h"
#include<string.h>

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100, RANK = 1,
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = INT_MAX, BUFFER_SIZE = 100;

/* structures for conducting tests*/
unsigned int MAX_BUFFER_SIZE[10], CNT_BUFFER_SIZE[10],
    AVG_BUFFER_SIZE[10], MIN_BUFFER_SIZE[10] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
                         RED_BUFFER_SIZE[10];

long int N;
char OUT_FILE[100];

double DECAY = 1.0, EPS = 0.0, THETA = 0.1, GAMMA = 2.0,
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01,
       H_FRACTION = 0.1, RATE_PARAMETER = 0.1, CARRY = 1.0,
       TIME_MINE = 1000.0;


int main()
{
    int sz, cnt, tid = 1, pattern = 0, no_patterns = 0, transactions = 0;
    long pos = 0, size;
    
    FILE *sf, *state;

    state = fopen(".state_1", "r");
    if (state == NULL)
    {
        state = fopen(".state_1", "w");
        fprintf(state, "%ld", pos);
    }

    fscanf(state, "%ld", &pos);
    if(pos == -1L)
    {
        printf("file has already been read!\n");
        fclose(state);
        exit(0);
    }

    state = fopen(".state_1", "w");

    printf("state = %ld\n", pos);

    int *test = (int*) malloc(10*sizeof(int));
    printf("sizeof test = %d\n", sizeof(test));

    sf = fopen("./data/5kD100T10.data", "r");
    fseek(sf, 0, SEEK_END);
    size = ftell(sf);
    rewind(sf);

    buffer stream = NULL, end = NULL;
    stream = (buffer) calloc(1, sizeof(struct buffer_node));
    // stream->itemset = (data) calloc(1, sizeof(struct data_node));
    // stream->itemset->next = NULL;
    stream->next = NULL;
    end = stream;
    printf("seek currently at %ld\n", ftell(sf));
    fseek(sf, pos, SEEK_SET);
    printf("seek now at %ld\n", ftell(sf));


    sforest forest = sf_create_sforest(); // initializing the forest and creating root nodes of all the trees.

    while (transactions < BATCH && fscanf(sf, "%d", &sz) != EOF)
    {
        // printf("transactions = %d, sz = %d\n", transactions, sz);
        data d = malloc((sz + 2) * sizeof(int));
        d[0] = 0;
        d[1] = 0;
        while(sz--)
        {
            data_type item;
            fscanf(sf, "%d", &item);
            d[d[1] + 2] = item;
            d[1]++;
        }

        d = sf_sort_data(d); // canonical sort of incoming trans
        printf("inserting transaction: ");
        sf_print_data_node(d);
        sf_prefix_insert_itemset(forest, d, 1, transactions);
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;

        end->freq = 0;
        end->next = NULL;
        transactions++;
    }
    sf_peel_tree(forest, -1);

    printf("(ftid: %d, ltid: %lf); freq = %lf --> ", end->ftid, end->ltid, end->freq);
    sf_print_data_node(end->itemset);

    printf("seek now at %ld\n", ftell(sf));
    if (size == ftell(sf))
    {
        printf("end of file2\n");
        fprintf(state, "%ld", -1L);
    }
    else
        fprintf(state, "%ld", ftell(sf)+1);
    fclose(state);
    fclose(sf);
    end = stream;
}