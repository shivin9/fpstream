#define GLOBAL_VARS 0
#include "sftree.h"
/*
    note: be careful with mining functions of cptree and sforest
    all are working correct but just that they need to be properly adjusted ie. sorting the sorted list for cptree and initializing all the arrays to 0 before using etc.

    CP and sf trees are working alright
*/

int BATCH = 1000,\
    DICT_SIZE = 100, HSIZE = 100,\
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3;

long int N;
char OUT_FILE[100];

double DECAY = 1.0, EPS = 0.0, THETA = 0.1,\
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01, CARRY = 1.0, H_FRACTION = 0.1;

/*DESCENDING order here*/
int cmpfunc (const void * a, const void * b)
{
   return -( *(double*)a - *(double*)b );
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
                -c<CARRY>\n\
                -t<THETA>\n\
                -(S/s)<SUP>\n\
                -m<min_sup_semifreq>\n\
                -M<min_sup_freq>\n\
                -L<LEAVE_LVL>\n\
                -H<SIZEOF_HASH_TABLE>\n\
                -p<pattern>\n");
        exit(-1);
    }

    FILE *sf;
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
                    case 'H': H_FRACTION =  strtof(s, &s);     break;
                    case 'S': SUP =  strtof(s, &s);            break;
                    case 'L': LEAVE_LVL =  strtod(s, &s);      break;
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
    HSIZE = H_FRACTION*DICT_SIZE;

    printf("\
            The parameters are:-\n\
            <DICT_SIZE>:        %d\n\
            <HSIZE>:            %d\n\
            <BATCH_SIZE>:       %d\n\
            <DECAY>:            %lf\n\
            <EPS>:              %lf\n\
            <CARRY>             %lf\n\
            <THETA>:            %lf\n\
            (S/s)<SUP>:         %lf\n\
            <LEAVE_LVL>:        %d\n",\
            DICT_SIZE, HSIZE, BATCH, DECAY, EPS, CARRY, THETA, SUP, LEAVE_LVL);

    srand(time(NULL));
    long unsigned size;
    sforest forest = NULL;

    forest = sf_create_sforest();

    // sftree tree = sf_create_sftree(0);
    // sf_create_header_table(tree, tid);

    struct timeval t1, t2, t3, t4;
    double elapsedTime, sum = 0, totaltime = 0, prune_time = 0;

    gettimeofday(&t1, NULL);


    buffer stream = NULL, end = NULL;
    stream = (buffer) calloc(1, sizeof(struct buffer_node));
    stream->itemset = barcreate((bit) DICT_SIZE);
    stream->next = NULL;
    end = stream;

    while(fscanf(sf, "%d", &sz) != EOF)
    {
        data d = barcreate((bit) DICT_SIZE);
        while(sz--)
        {
            data_type item;
            fscanf(sf, "%d", &item);
            // printf("%d ", item);
            barset(d, (bit)item);
        }
        // printf("\n");
        batch_no++;
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        end->next = NULL;
    }
    fclose(sf);
    end = stream;
    stream = stream->next;
    sf_delete_data_node(end->itemset);
    free(end);

    while(stream)
    {

        gettimeofday(&t3, NULL);
        sf_insert_itemset(forest, stream->itemset, tid);
        gettimeofday(&t4, NULL);
        // printf("inserting: ");
        // sf_print_data_node(stream->itemset);
        // sf_fp_insert(tree->root, tree->head_table, d->next, tid);

        elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
        elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
        totaltime += elapsedTime;

        end = stream->next;
        sf_delete_data_node(stream->itemset);
        free(stream);
        stream = end;

        // sf_prune(forest, tid);
        // break;
        if(tid%BATCH == 0)
        {
            printf("pruning at tid = %d\n", tid);
            gettimeofday(&t3, NULL);
            // sf_empty_buffers(forest, tid);
            // sf_prune(forest, tid);
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

    printf("total time taken to insert in sf tree = %lf ms\n", elapsedTime);
    printf("average time to insert in sf tree = %lf ms\n", totaltime/tid);

    printf("total intermittent prune time = %lf ms\n", prune_time);
    printf("avg. intermittent prune time = %lf ms\n", prune_time/(N/BATCH));

    // sf_print_sforest(forest);

    gettimeofday(&t3, NULL);
    sf_empty_buffers(forest, tid);
    gettimeofday(&t4, NULL);

    // sf_print_sforest(forest);

    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    printf("total time taken to empty the buffers = %lf ms\n", elapsedTime);

    gettimeofday(&t1, NULL);
    no_patterns = sf_mine_frequent_itemsets(forest, tid, pattern);
    gettimeofday(&t2, NULL);

    // sfnode collector = calloc(1, sizeof(struct sf_node));

    // sf_print_tree(tree->root);
    // printf("****printing Htable****\n");
    // sf_print_header_table(tree->head_table);
    /* testing the sf_dfs() function*/
    // sftree condtree = sf_create_conditional_sf_tree(tree, 3, MINSUP_SEMIFREQ, 0);
    // sf_print_tree(condtree->root);

    // sf_fp_mine_frequent_itemsets(tree, sorted, NULL, collector, tid, MINSUP_SEMIFREQ);
    // sf_print_patterns_to_file(NULL, collector->bufferhead, /*cnt = */ -1, -1, 0);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    printf("(%d items) total time taken to mine the sf tree = %lf ms\n",\
            no_patterns, elapsedTime);

    {
        // sftree test = sf_create_sftree(0);
        // data temp = sorted;
        // while(temp)
        // {
        //     sf_append_buffer(test->root, temp, 1, tid); /* push back the popped buffer as we have to leave now*/
        //     temp = temp->next;
        // }
        // // sf_delete_buffer_table(test->root->hbuffer);
        // for(i = 0; i < 200; i++)
        // {
        //     buffer popped = sf_pop_buffer(test->root, rand()%HSIZE, tid);
        //     sf_delete_buffer(popped);
        // }
        // sf_print_buffer_table(test->root->hbuffer);
        // if(test->root->hbuffer != NULL)
        // {
        //     for(i = 0; i < 10; i++)
        //     {
        //         assert(test->root->hbuffer[i] != NULL);
        //     }
        // }
        // sf_print_node(test->root);
        // sf_delete_sftree(test);
    }

    sf_delete_sforest(forest);
    free(forest);
    return 0;
}