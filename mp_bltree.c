#define GLOBAL_VARS 0

#include "sfstream.h"

/* This code was originally for making SWP-tree an anytime algorithm*/

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100,
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3, BUFFER_SIZE = 100;

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

timeval origin, global_timer;

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        fprintf(stdout, "format is ./exe <INPUT_filename><OUTPUT_filename>\n\
                -D<DICT_SIZE>\n\
                -B<BATCH_SIZE>\n\
                -d<DECAY>\n\
                -e<EPS>\n\
                -c<CARRY>\n\
                -t<THETA>\n\
                -T<TIME_MINE>\n\
                -g<GAMMA>\n\
                -(S/s)<SUP>\n\
                -m<min_sup_semifreq>\n\
                -M<min_sup_freq>\n\
                -L<LEAVE_LVL>\n\
                -B<BUFFER_SIZE>\n\
                -H<SIZEOF_HASH_TABLE>\n\
                -p<pattern>\n");
        exit(-1);
    }

    FILE *sf, *poisson; // files to read the data set and poisson values
    sf = fopen("intermediate", "w");
    fclose(sf);
    sf = fopen("output", "w");
    fclose(sf);
    
    // sfstream(argv[1]);
    strcpy(OUT_FILE, argv[2]); // the name of the output file is being copied.
    
    char *s, output;
    
    int i, batch_no = 0, stream_batch = 0, sleepTime = 100, tree_to_prune, sz, batch_size, cnt,\
        item_ready, leavecnt = 1, tid = 1, pattern = 0, no_patterns = 0, curr_tree;
    
    int leave_as_buffer;
    
    for (i = 3; i < argc; i++)
    {                /* traverse arguments */
        s = argv[i]; /* get option argument */
        if ((*s == '-') && *++s)
        { /* -- if argument is an option */
            while (*s)
            { /* traverse options */
                switch (*s++)
                { /* evaluate switches */
                case 'd':
                    DECAY = strtof(s, &s);
                    break;
                case 'e':
                    EPS = strtof(s, &s);
                    break;
                case 'c':
                    CARRY = strtof(s, &s);
                    break;
                case 't':
                    THETA = strtof(s, &s);
                    break;
                case 'T':
                    TIME_MINE = strtof(s, &s);
                    break;
                case 's':
                    SUP = strtof(s, &s);
                    break;
                case 'm':
                    MINSUP_SEMIFREQ = strtof(s, &s);
                    break;
                case 'M':
                    MINSUP_FREQ = strtof(s, &s);
                    break;
                case 'B':
                    BATCH = strtod(s, &s);
                    break;
                case 'b':
                    BUFFER_SIZE = strtod(s, &s);
                    break;
                case 'p':
                    pattern = strtod(s, &s);
                    break;
                case 'D':
                    DICT_SIZE = strtod(s, &s);
                    break;
                case 'H':
                    H_FRACTION = strtof(s, &s);
                    break;
                case 'g':
                    GAMMA = strtof(s, &s);
                    break;
                case 'S':
                    SUP = strtof(s, &s);
                    break;
                case 'L':
                    LEAVE_LVL = strtod(s, &s);
                    break;
                case 'r':
                    RATE_PARAMETER = strtof(s, &s);
                    break;
                default:
                    fprintf(stdout, "UNKNOWN ARGUMENT! %c", *(s - 1));
                    exit(-1);
                    break;
                }
            }
        }
    }

    sf = fopen(OUT_FILE, "w");
    fclose(sf);

    sf = fopen(argv[1], "r");
    if (sf == NULL)
    {
        fprintf(stdout, "invalid file\n");
        exit(0);
    }
    HSIZE = H_FRACTION * DICT_SIZE; // size of the hash table computed here

    fprintf(stdout, "\
            The parameters are:-\n\
            <DICT_SIZE>:        %d\n\
            <HSIZE>:            %d\n\
            <BATCH_SIZE>:       %d\n\
            <BUFFER_SIZE>:      %d\n\
            <DECAY>:            %lf\n\
            <EPS>:              %lf\n\
            <RATE_PARAMETER>:   %lf\n\
            <CARRY>             %lf\n\
            <GAMMA>             %lf\n\
            <THETA>:            %lf\n\
            <TIME_MINE>:        %lf\n\
            (S/s)<SUP>:         %lf\n\
            <LEAVE_LVL>:        %d\n",
            DICT_SIZE, HSIZE, BATCH, BUFFER_SIZE,
            DECAY, EPS, RATE_PARAMETER,
            CARRY, GAMMA, THETA, TIME_MINE, SUP,
            LEAVE_LVL);
            
    srand(time(NULL));
    poisson = fopen("poisson.ignore", "r");
    
    // long unsigned size;
    sforest forest1 = NULL, forest2 = NULL, temp = NULL;
    forest1 = sf_create_sforest(); // initializing the forest1 and creating root nodes of all the trees.
    forest2 = sf_create_sforest(); // initializing the forest2 and creating root nodes of all the trees.
    
    patterntree ptree = create_pattern_tree();

    pfptree aux = NULL;
    
    struct timeval t1, t2, t3, t4;
    double elapsedTime, sum = 0, totaltime = 0, prune_time = 0, insertionTime = 0, delay_time;
    
    gettimeofday(&t1, NULL);
    gettimeofday(&origin, NULL);
    // gettimeofday(&global_timer, NULL);

    // initializing the first LL of stream separately. Buffer structure is being re-used here.
    buffer stream = NULL, end = NULL, curr = NULL;
    stream = (buffer)calloc(1, sizeof(struct buffer_node));
    // stream->itemset = (data) calloc(1, sizeof(struct data_node));
    // stream->itemset->next = NULL;
    stream->next = NULL;
    end = stream;

    while (fscanf(sf, "%d", &sz) != EOF)
    {
        data d = malloc((sz + 2) * sizeof(int));
        d[0] = 0;
        d[1] = 0;
        while (sz--)
        {
            data_type item;
            fscanf(sf, "%d", &item);
            d[d[1] + 2] = item;
            d[1]++;
            batch_no++;
        }

        d = sf_sort_data(d); // canonical sort of incoming trans
        // sf_print_data_node(d);
        end->next = (buffer)calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        fscanf(poisson, "%lf", &delay_time);
        end->freq = delay_time / RATE_PARAMETER; /* this time is in milli-seconds, reusing the freq field to store delay_time*/
        end->next = NULL;
    }
    
    fclose(sf);
    end = stream;
    stream = stream->next;
    curr = stream;

    // sf_print_buffer(stream);
    sf_delete_data_node(end->itemset);
    // deleting the head of itemset LL which was dummy.
    free(end);
    printf("Read the Stream...\n");
    /* Stream has been read */

    gettimeofday(&t1, NULL);
    omp_set_num_threads(4); /* 2 streams, 1 pattern tree and 1 stream manager*/

    curr_tree = 0;
    T1 = 0;
    T2 = 0;
    tree_to_prune =-1;
    data curr_itemset;
    batch_ready = 0;
    item_ready = -1;

    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        if(threadId == 0) // master thread
        {
            // 1. Reads the data and pumps it into the stream with poisson distribution.
            // 2. After reading one transcation is complete, it simply notifies the current worker thread to pre-empt from it's work process the newly read transaction.
            // 3. After the worker thread has taken away the transaction, it reads next transaction from the file and pumps it again with poisson distribution.

            cnt = 1;
            // while(1)
            // {
                while(cnt <= batch_no)
                {
                    // # pragma omp critical
                    // {
                        item_ready = 1;
                        // printf("Releasing Item no. %d\n", cnt);
                        leave_as_buffer = 0;
/*                         sleepTime = rand()%100;
                        cnt += leavecnt;
                        usleep(sleepTime); // not needed as time to insert is supplied in the stream
*/
                         item_ready = 0;
                    // }
                }
            // }
            printf("Released All Items\n");
            item_ready = -1;
        }

        else if (threadId == 1) // worker thread 1 on AnyFPStream
        {
            // 1. Waits in an infinite loop until a notification comes from the master thread for a new transaction.
            // 2. Once notification comes, it tries to insert into the CP-Tree
            // 3. If in between the insertions, a notification of new transaction comes, it pre-emtps and processes the new transaction.
            // 4. While descending the CP-tree it takes care of carrying forward to the next level accumulated buffers.
            // 5. After the stream has switched, you process the tree and notify the thread with ID 3.
            // 6. All other aspects of handling the CP-tree. (Yet to complete)
            do{
                // printf("stream_batch = %d, batch_no = %d\n", stream_batch, batch_no);
                if(stream_batch >= batch_no && T2 == 1)
                    break;

                while(item_ready == 0 && stream_batch < cnt)
                {
                    // printf("stuck in T1\n");
                }
                if(stream_batch < cnt && curr_tree == 0 && T1 == 1)
                {
                    // usleep(sleepTime*10);
                    // printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 1, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    // printf("leave_as_buffer before = %d\n", leave_as_buffer);
                    // assert(ftree->head_table != NULL);
                    gettimeofday(&t3, NULL);
                    sf_insert_itemset(forest1, curr->itemset, tid, curr->freq, &t3);
                    // printf("leave_as_buffer after = %d\n", leave_as_buffer);
                    stream_batch++;

                    if(curr->next)
                    {
                        stream = curr;
                        free(curr->itemset);
                        curr = curr->next;
                        free(stream);
                    }

                    /* prune the tree if 5000 transactions have been inserted or if the stream has ended */
                    if(T2 == 1 && (stream_batch%BATCH == 0 || stream_batch >= batch_no))
                    {
                        // direct away the stream
                        printf("\nCHANGING TREE 1->2; count = %d\n", stream_batch);
                        #pragma omp critical
                        {
                            curr_tree = 1;
                            T1 = 0;
                        }
                        sf_prune(forest1, tid);
                        printf("finished pruning FOREST_1\n");
                        T1 = 1;
                    }
                }
            }while(1);
        }

        else if(threadId == 2)
        {
            do{
                // printf("stream_batch = %d, batch_no = %d\n", stream_batch, batch_no);
                /* this is to break from the infinite loop */
                if(stream_batch >= batch_no && T2 == 1)
                    break;
                while(item_ready == 0 && stream_batch < cnt)
                {
                    // printf("stuck in T2\n");
                }
                if(stream_batch < cnt && curr_tree == 1  && T2 == 1)
                {
                    printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 2, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    gettimeofday(&t3, NULL);
                    sf_insert_itemset(forest2, curr->itemset, tid, curr->freq, &t3);

                    stream_batch++;
                    if(curr->next)
                    {
                        stream = curr;
                        free(curr->itemset);
                        curr = curr->next;
                        free(stream);
                    }

                    if (T1 == 1)
                    {
                        printf("\nCHANGING TREE 2->1; count = %d\n", stream_batch);
                        // direct away the stream
                        #pragma omp critical
                        {
                            curr_tree = 0;
                            T2 = 0;
                        }
                        printf("finished converting FOREST_2\n");
                        T2 = 1;
                    }
                }
            }while(1);
        }

        else if (threadId == 3) // thread handling pattern tree along with
        {
            /* 1. When we receive a notification from one of the worker threads,
                  insert the extracted FIs into the pattern tree. */
            do{
                if(T1 == 0 || T2 == 0){
                    tree_to_prune = 1 - curr_tree;
                    // printf("Servicing TREE_%d, batch_ready=%d\n", tree_to_prune + 1, batch_ready);

                    /* we will the forest which is full */
                    if(tree_to_prune)
                        temp = forest1;
                    else
                        temp = forest2;

                    # pragma omp critical
                    {
                        no_patterns = sf_mine_frequent_itemsets\
                                      (temp, tid, pattern); // mining for frequent patterns

                        process_batch(ptree, ++batch_ready);
                        // printf("Done Servicing TREE_%d\n\n", tree_to_prune + 1);

                        aux = fp_create_fptree();
                        if(tree_to_prune)
                            T2 = 0;
                        else
                            T1 = 0;
                    }

                    FILE *fp1;
                    fp1 = fopen("output", "a");
                    fprintf(fp1, "After batch %d:\n",batch_ready);
                    fclose(fp1);

                    aux = get_fptree(ptree);
                    // sf_fp_mine_frequent_itemsets(aux, 0,\
                    //                             data till_now, bufferTable collected, tid, double minsup);
                    fp_delete_fptree(aux);
                }
            }while(stream_batch <= batch_no);
        }
        # pragma omp barrier
        # pragma omp join
    }

    N = tid;
    /* this is to accomodate hard support counts instead of % */
    if (SUP > 1.0)
    {
        // if(SUP/N - EPS < 0) /* mine with very less support */
        SUP = SUP / N;
        // else
        //     SUP = SUP/N - EPS;
    }
    // else
    // {
    //     if(SUP - EPS > 0)
    //         SUP = SUP - EPS;
    // }

    gettimeofday(&t2, NULL);

/* 
    for (i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "MAX_BUFFER_SIZE[%d] = %u\n", i, MAX_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

    for (i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "MIN_BUFFER_SIZE[%d] = %u\n", i, MIN_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

    for (i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "AVG_BUFFER_SIZE[%d] = %lf\n", i, (float)AVG_BUFFER_SIZE[i] / CNT_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

    for (i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "RED_BUFFER_SIZE[%d] = %lf\n", i, (float)RED_BUFFER_SIZE[i] / CNT_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    fprintf(stdout, "total time taken to insert in sf tree = %lf ms, buffered = %d\n", elapsedTime, buffered);
    fprintf(stdout, "average time to insert in sf tree = %lf ms\n", insertionTime / tid);

    fprintf(stdout, "total intermittent prune time = %lf ms\n", prune_time);
    fprintf(stdout, "avg. intermittent prune time = %lf ms\n", prune_time / (N / BATCH));

 */

    gettimeofday(&t3, NULL);
    sf_prune(forest1, tid); // final pruning before emptying the buffers
    sf_prune(forest2, tid); // final pruning before emptying the buffers

    sf_empty_buffers(forest1, tid, TIME_MINE);
    sf_empty_buffers(forest2, tid, TIME_MINE);
    gettimeofday(&t4, NULL);

    // sf_print_sforest(forest);
    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    fprintf(stdout, "total time taken to empty/prune the buffers = %lf ms\n", elapsedTime);

    gettimeofday(&t1, NULL);
    no_patterns += sf_mine_frequent_itemsets(forest1, tid, pattern); // mining for frequent patterns
    no_patterns += sf_mine_frequent_itemsets(forest2, tid, pattern); // mining for frequent patterns
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    fprintf(stdout, "(%d freq. patterns) total time taken to mine the sf tree = %lf ms\n",
            no_patterns, elapsedTime);

    fflush(stdout);
    // to do final free
    // sf_delete_sforest(forest);
    // free(forest);
    // // sf_delete_data_node(sorted);
    return 0;
}
