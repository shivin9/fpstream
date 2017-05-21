#define GLOBAL_VARS 0
#include "sftree.h"
/*
    note: be careful with mining functions of cptree and sforest
    all are working correct but just that they need to be properly adjusted ie. sorting the sorted list for cptree and initializing all the arrays to 0 before using etc.

    CP and sf trees are working alright
*/

int BATCH = 1000,\
    DICT_SIZE = 100, HSIZE = 10000,\
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3;

long int N;
char OUT_FILE[100];

double DECAY = 1.0, EPS = 0.0, THETA = 0.1,\
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01,\
       CARRY = 1.0, PRUNE_TIME = 0, RATE_PARAMETER = 0.01;

/*DESCENDING order here*/
int cmpfunc (const void * a, const void * b)
{
   return -( *(double*)a - *(double*)b );
}

double nextTime(double rateParameter)
{
    return -logf(1.0f - (double) random() / RAND_MAX) / rateParameter;
}


int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        printf("format is ./exe <INPUT_filename><OUTPUT_filename>\n\
                -D<DICT_SIZE>\n\
                -B<BATCH_SIZE>\n\
                -d<DECAY>\n\
                -e<EPS>\n\
                -r<RATE_PARAMETER>\n\
                -c<CARRY>\n\
                -t<THETA>\n\
                -(S/s)<SUP>\n\
                -m<min_sup_semifreq>\n\
                -M<min_sup_freq>\n\
                -L<LEAVE_LVL>\n\
                -p<pattern>\n");
        exit(-1);
    }

    FILE *sf, *poisson;
    int sz, cnt, max = 0, tid = 1, pattern = 0, no_patterns = 0;
    sf = fopen("intermediate", "w");
    fclose(sf);
    sf = fopen("output", "w");
    fclose(sf);

    // sfstream(argv[1]);
    strcpy(OUT_FILE, argv[2]);

    char* s, output;

    int i, batch_no;
    for (i = 3; i < argc; i++)
    {                               /* traverse arguments */
        s = argv[i];                /* get option argument */
        if ((*s == '-') && *++s)
        {                       /* -- if argument is an option */
            while (*s)
            {                       /* traverse options */
                switch (*s++)
                {                   /* evaluate switches */
                    case 'd': DECAY  = strtof(s, &s);          break;
                    case 'e': EPS    = strtof(s, &s);          break;
                    case 'c': CARRY  = strtof(s, &s);          break;
                    case 't': THETA  = strtof(s, &s);          break;
                    case 's': SUP    = strtof(s, &s);          break;
                    case 'm': MINSUP_SEMIFREQ = strtof(s, &s); break;
                    case 'M': MINSUP_FREQ = strtof(s, &s);     break;
                    case 'B': BATCH  =       strtod(s, &s);    break;
                    case 'p': pattern  =   strtod(s, &s);      break;
                    case 'D': DICT_SIZE =  strtod(s, &s);      break;
                    case 'S': SUP =  strtof(s, &s);            break;
                    case 'L': LEAVE_LVL =  strtod(s, &s);      break;
                    case 'r': RATE_PARAMETER =  strtof(s, &s); break;
                    default : printf("UNKNOWN ARGUMENT! %c", *(s-1));
                              exit(-1);                        break;
                }
            }
        }
    }

    sf = fopen(OUT_FILE, "w");
    fclose(sf);

    sf = fopen(argv[1], "r");
    if(sf == NULL)
    {
        printf("invalid file\n");
        exit(0);
    }

    printf("\
            The parameters are:-\n\
            <DICT_SIZE>:        %d\n\
            <BATCH_SIZE>:       %d\n\
            <DECAY>:            %lf\n\
            <EPS>:              %lf\n\
            <RATE_PARAMETER>:   %lf\n\
            <CARRY>             %lf\n\
            <THETA>:            %lf\n\
            (S/s)<SUP>:         %lf\n\
            <LEAVE_LVL>:        %d\n",\
            DICT_SIZE, BATCH, DECAY, EPS, RATE_PARAMETER, CARRY, THETA, SUP, LEAVE_LVL);

    srand(time(NULL));
    poisson = fopen("poisson.ignore", "r");

    long unsigned size;
    sforest forest = NULL;

    data sorted = sf_create_sorted_dummy(0); // list of items in sorted order, used in mining. Not required.
    forest = sf_create_sforest(); // creates an empty forest and initializes all the root nodes of the tree.

    // fptree tree = sf_create_fptree(0);
    // sf_create_header_table(tree, tid);

    timeval t1, t2, t3, t4;
    double elapsedTime, sum = 0, insertionTime = 0, prune_time = 0, delay_time = 0;

    gettimeofday(&t1, NULL);

    buffer stream = NULL, end = NULL; // initializing first node of transactions list
    stream = (buffer) calloc(1, sizeof(struct buffer_node));
    stream->itemset = (data) calloc(1, sizeof(struct data_node));
    stream->itemset->next = NULL;
    stream->next = NULL;
    end = stream;

    while(fscanf(sf, "%d", &sz) != EOF) // reading the stream into a linked list of linked lists
    {
        data d = NULL; // filled d here
        while(sz--) // reading sz items in a loop
        {
            data_type item;
            fscanf(sf, "%d", &item);
            data new_d = calloc(1, sizeof(struct data_node));
            if(new_d == NULL)
            {
                printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
        }

        batch_no++;  // incrementing the transaction number
        sf_sort_data(d, NULL); // sorts the linked list of items in canonical ordering, by passing NULL.

        // inserting d at the end and updating end. Traverse and then insert
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        fscanf(poisson, "%lf", &delay_time);
        end->freq = delay_time / RATE_PARAMETER; /* this time is in milli-seconds*/
        end->next = NULL;
    }
    fclose(sf);
    end = stream;
    stream = stream->next;
    sf_delete_data_node(end->itemset); // deleting the first node (sentinel) at the end of reading the file
    free(end);

    // file is read, stream is ready to take insertions.

    while(stream)
    {

        gettimeofday(&t3, NULL);
        sf_insert_itemset(forest, stream->itemset, tid, stream->freq, &t3); // insert the transaction into the BL forest.
        gettimeofday(&t4, NULL);

        // sf_fp_insert(tree->root, tree->head_table, d->next, tid);

        elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
        elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
        insertionTime += elapsedTime;

        // sf_create_header_table_helper(forest->root, forest->head_table);
        // sf_update_header_table(forest->head_table, d, tid);
        // sf_print_tree(forest->root);
        // traversing the stream ahead and freeing the previous node.
        end = stream->next;
        sf_delete_data_node(stream->itemset);
        free(stream);
        stream = end;

        // sf_prune(forest, tid);
        // break;
        // intermittent pruning
        if(tid%BATCH == 0)
        {
            printf("pruning at tid = %d\n", tid);
            gettimeofday(&t3, NULL);
            // sf_empty_buffers(forest, tid);
            sf_prune(forest, tid); // intermittent pruning taking place here.
            gettimeofday(&t4, NULL);
            elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
            elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
            prune_time += elapsedTime;
        }
        tid++;
    }

    /* Create the perfect, final tree after emptying the buffers*/
    // sf_empty_buffers(forest, forest, tid);
    // sf_prune(forest, tid);
    N = tid;
    /* this is to accomodate hard support counts instead of %*/
    if(SUP > 1.0)
    {
        if(SUP/N - EPS < 0) /* mine with very less support*/
            SUP = SUP/N;
        else
            SUP = SUP/N - EPS;
    }
    else
    {
        if(SUP - EPS > 0)
            SUP = SUP - EPS;
    }

    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    // sf_print_sforest(forest);
    
    printf("total time taken to insert in sf tree = %lf ms\n", insertionTime);
    printf("average time to insert in sf tree = %lf ms\n", insertionTime/tid);

    printf("total intermittent prune time = %lf ms\n", prune_time);
    printf("avg. intermittent prune time = %lf ms\n", prune_time/(N/BATCH));

    printf("total INSERTION PRUNE_TIME = %lf ms\n", PRUNE_TIME);


    gettimeofday(&t1, NULL);
    
    struct timespec tms3, tms4;
    
    clock_gettime(CLOCK_REALTIME,&tms3);
    gettimeofday(&t3, NULL);
    sf_empty_buffers(forest, tid); // empty the buffers before pruning
    gettimeofday(&t4, NULL);
    clock_gettime(CLOCK_REALTIME,&tms4);
    
    sf_prune(forest, tid); // final pruning before emptying the buffers
    no_patterns = sf_mine_frequent_itemsets(forest, tid, pattern);
    gettimeofday(&t2, NULL);

    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    printf("total time taken to empty the buffers = %lf ms\n", elapsedTime);

    elapsedTime = (tms4.tv_sec - tms3.tv_sec) * 1000000.0;
    elapsedTime += (tms4.tv_nsec - tms3.tv_nsec) / 1000000.0;
    printf("total time taken to empty the buffers (tms) = %lf ms\n", elapsedTime);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    printf("(%d items) total time taken to mine the sf tree = %lf ms\n",\
            no_patterns, elapsedTime);

    // sf_delete_sftree(tree);
    // sf_delete_sforest(forest);
    // free(forest);
    // sf_delete_data_node(sorted);
    return 0;
}