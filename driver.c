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
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01;

QStack* mem_bin;

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
                -t<THETA>\n\
                -(S/s)<SUP>\n\
                -m<min_sup_semifreq>\n\
                -M<min_sup_freq>\n\
                -L<LEAVE_LVL>\n\
                -p<pattern>\n");
        exit(-1);
    }

    FILE *sf;
    int sz, cnt, max = 0, tid = 1, pattern = 0;
    sf = fopen("intermediate", "w");
    fclose(sf);
    sf = fopen("output", "w");
    fclose(sf);

    // sfstream(argv[1]);
    strcpy(OUT_FILE, argv[2]);

    char* s, output;

    int i;
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
                    case 't': THETA  = strtof(s, &s);          break;
                    case 's': SUP    = strtof(s, &s);          break;
                    case 'm': MINSUP_SEMIFREQ = strtof(s, &s); break;
                    case 'M': MINSUP_FREQ = strtof(s, &s);     break;
                    case 'B': BATCH  =       strtod(s, &s);    break;
                    case 'p': pattern  =   strtod(s, &s);      break;
                    case 'D': DICT_SIZE =  strtod(s, &s);      break;
                    case 'S': SUP =  strtod(s, &s);            break;
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


    struct timeval t1, t2, t3, t4, ts, tf;
    double elapsedTime, sum = 0, totaltime = 0;

    gettimeofday(&ts, NULL);
    
    long unsigned size;
    sforest forest = NULL;

    data sorted = sf_create_sorted_dummy(0);
    forest = sf_create_sforest();
    mem_bin = createQStack();
    
    sftree tree = sf_create_sftree(0);
    sf_create_header_table(tree, tid);

    gettimeofday(&t1, NULL);

    while(fscanf(sf, "%d", &sz) != EOF)
    {
        data d = NULL;
        while(sz--)
        {
            data_type item;
            fscanf(sf, "%d", &item);

            data new_d = malloc(sizeof(struct data_node));
            if(new_d == NULL)
            {
                printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
        }
        /* removes duplicates items also*/
        // printf("inserting: ");
        sf_sort_data(d, NULL);
        // sf_print_data_node(d);

        gettimeofday(&t3, NULL);
        sf_insert_itemset(forest, d, tid);
        gettimeofday(&t4, NULL);

        // sf_fp_insert(tree->root, tree->head_table, d->next, tid);

        elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
        elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
        totaltime += elapsedTime;

        // sf_create_header_table_helper(forest->root, forest->head_table);
        // sf_update_header_table(forest->head_table, d, tid);
        // sf_print_tree(forest->root);
        sf_delete_data_node(d);
        // sf_prune(forest, tid);
        // break;
        
        if(mem_bin->size > 1000)
            sf_clear_garbage();

        if(tid%BATCH == 0)
        {
            sf_prune(forest, tid);
        }
        tid++;
    }
    fclose(sf);

    /* Create the perfect, final tree after emptying the buffers*/
    // sf_empty_buffers(forest, forest, tid);
    // sf_prune(forest, tid);
    gettimeofday(&t2, NULL);
    N = tid;
    /* this is to accomodate hard support counts instead of %*/
    if(SUP > 1.0)
        SUP = SUP/N;


    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to insert in sf tree = %lf ms\n", elapsedTime);
    // sf_print_sforest(forest);
    // sf_print_sforest_lvl(forest);
    // printf("sizeof sf tree = %lf\n", sf_size_of_sforest(forest));

    printf("average time to insert in sf tree = %lf ms\n", totaltime/tid);

    gettimeofday(&t1, NULL);
    sf_mine_frequent_itemsets(forest, tid, pattern);
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
    printf("total time taken to mine the sf tree = %lf ms\n", elapsedTime);

    // int idx = 0, bnode=50;
    // cnt = 0;
    // // for(bnode = 0; bnode < DICT_SIZE; bnode++)
    // {
    //     cnt = 0;
    //     for(idx = 0; idx < last_index(50); idx++)
    //     {
    //         cnt += forest[bnode]->root->children[idx] ? 1 : 0;
    //         printf("child = %d\n", forest[bnode]->root->children[idx]->data_item);
    //     }
    // }

    sf_clear_garbage();
    sf_delete_sftree(tree);
    sf_delete_sforest(forest);
    free(forest);
    sf_delete_data_node(sorted);

    gettimeofday(&tf, NULL);

    elapsedTime = (tf.tv_sec - ts.tv_sec) * 1000.0;
    elapsedTime += (tf.tv_usec - ts.tv_usec) / 1000.0;
    printf("TOTAL time taken = %lf ms\n", elapsedTime);
    return 1;
}