<<<<<<< HEAD
#include "fpstream.h"
/*
    note: be careful with mining functions of cptree and fptree
    all are working correct but just that they need to be properly adjusted ie. sorting the sorted list for cptree and initializing all the arrays to 0 before using etc.
*/
// call this function to start a nanosecond-resolution timer

// long timer_end(struct timespec start_time)
// {
//     struct timespec end_time = current_kernel_time();
//     return(end_time.tv_nsec - start_time.tv_nsec);
// }

// struct timespec timer_start(void)
// {
//     return current_kernel_time();
// }


// struct timespec timer_start(){
//     struct timespec start_time;
//     getrawmonotonic(&start_time);
//     // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
//     return start_time;
// }

// // call this function to end a timer, returning nanoseconds elapsed as a long
// long timer_end(struct timespec start_time){
//     struct timespec end_time;
//     getrawmonotonic(&end_time);
//     // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
//     long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
//     return diffInNanos;
// }
=======
#define GLOBAL_VARS 0
#include "sftree.h"

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100,\
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3, BUFFER_SIZE = 100;
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746

/* structures for conducting tests*/
unsigned int MAX_BUFFER_SIZE[10], CNT_BUFFER_SIZE[10],\
             AVG_BUFFER_SIZE[10], MIN_BUFFER_SIZE[10] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},\
             RED_BUFFER_SIZE[10];

long int N;
char OUT_FILE[100];

<<<<<<< HEAD
int CNT = 0;
=======
double DECAY = 1.0, EPS = 0.0, THETA = 0.1, GAMMA=2.0,\
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01,\
       H_FRACTION = 0.1, RATE_PARAMETER = 0.1, CARRY = 1.0,\
       TIME_MINE = 1000.0;

timeval origin, global_timer;
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
<<<<<<< HEAD
        printf("Please enter filename!\n");
        exit(-1);
    }

    FILE *fp;

    struct timeval t1, t2, tp1, tp2, tt1, tt2;
    double elapsedTime = 0, pruneTime = 0;

    fp = fopen("intermediate", "w");
    fclose(fp);
    fp = fopen("output", "w");
    fclose(fp);
=======
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
    int sz, cnt, tid = 1, pattern = 0, no_patterns = 0;
    sf = fopen("intermediate", "w");
    fclose(sf);
    sf = fopen("output", "w");
    fclose(sf);

    // sfstream(argv[1]);
    strcpy(OUT_FILE, argv[2]); // the name of the output file is being copied.

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
                    case 'T': TIME_MINE  = strtof(s, &s);      break;
                    case 's': SUP    = strtof(s, &s);          break;
                    case 'm': MINSUP_SEMIFREQ = strtof(s, &s); break;
                    case 'M': MINSUP_FREQ = strtof(s, &s);     break;
                    case 'B': BATCH  =       strtod(s, &s);    break;
                    case 'b': BUFFER_SIZE  = strtod(s, &s);    break;
                    case 'p': pattern  =   strtod(s, &s);      break;
                    case 'D': DICT_SIZE =  strtod(s, &s);      break;
                    case 'H': H_FRACTION =  strtof(s, &s);     break;
                    case 'g': GAMMA      =  strtof(s, &s);     break;
                    case 'S': SUP =  strtof(s, &s);            break;
                    case 'L': LEAVE_LVL =  strtod(s, &s);      break;
                    case 'r': RATE_PARAMETER =  strtof(s, &s); break;
                    default : fprintf(stdout, "UNKNOWN ARGUMENT! %c", *(s-1));
                              exit(-1);                        break;
                }
            }
        }
    }
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746

    sf = fopen(OUT_FILE, "w");
    fclose(sf);

<<<<<<< HEAD
    if(fp == NULL)
    {
        printf("invalid file\n");
=======
    sf = fopen(argv[1], "r");
    if(sf == NULL)
    {
        fprintf(stdout, "invalid file\n");
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746
        exit(0);
    }
    HSIZE = H_FRACTION*DICT_SIZE; // size of the hash table computed here

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
            <LEAVE_LVL>:        %d\n",\
            DICT_SIZE, HSIZE, BATCH, BUFFER_SIZE,\
            DECAY, EPS, RATE_PARAMETER,\
            CARRY, GAMMA, THETA, TIME_MINE, SUP,\
            LEAVE_LVL);

    srand(time(NULL));
    poisson = fopen("poisson.ignore", "r");

    // long unsigned size;
    sforest forest = NULL;

    // data sorted = sf_create_sorted_dummy(0);
    forest = sf_create_sforest(); // initializing the forest and creating root nodes of all the trees.

    // sftree tree = sf_create_sftree(0);
    // sf_create_header_table(tree, tid);

    struct timeval t1, t2, t3, t4;
    double elapsedTime, sum = 0, totaltime = 0, prune_time = 0, insertionTime = 0, delay_time;

<<<<<<< HEAD
    int sz;
    fptree ftree = fp_create_fptree();

    data sorted = create_sorted_dummy();
    patterntree ptree = NULL;
    ptree = create_pattern_tree();

    buffer stream = NULL, end = NULL, curr = NULL;
    stream =  (buffer) malloc(sizeof(struct buffer_node));
    stream->itemset = (data) malloc(sizeof(struct data_node));
    stream->itemset->next = NULL;
    stream->next = NULL;
    curr = stream;
    end = stream;

    while(fscanf(fp, "%d", &sz) != EOF)
    {
        data d = NULL;
        while(sz--)
        {
            data_type item;
            fscanf(fp, "%d", &item);

            data new_d = malloc(sizeof(struct data_node));
            if(new_d == NULL)
            {
                //printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
            // sleepTime = 2000;
            // usleep(sleepTime);
        }

        fp_sort_data(d, NULL);
        end->next = (buffer) malloc(sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        end->next = NULL;
    }
    end->next = NULL;
    curr = curr->next;

    gettimeofday(&tt1, NULL);
    while(curr)
    {
        if(CNT < 0 && CNT % BATCH_SIZE == 0)
        {
            printf("pruning at tid = %d\n", CNT);
            gettimeofday(&tp1, NULL);
            fp_create_header_table(ftree);
            fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
            process_batch(ptree, CNT/BATCH_SIZE);
            gettimeofday(&tp2, NULL);

            pruneTime += (tp2.tv_sec - tp1.tv_sec)*1000 + 
              ((tp2.tv_usec - tp1.tv_usec)/1000.0);

            /* This code is if we want to print results after every batch
            FILE *fp1;
            fp1 = fopen("output", "a");
            fprintf(fp1, "After batch %d:\n", CNT/BATCH_SIZE);
            fclose(fp1);
            */
            fp_delete_fptree(ftree);
            ftree = get_fptree(ptree);
            fp_mine_frequent_itemsets(ftree, sorted, NULL, 1);
            fp_delete_fptree(ftree);
            ftree = fp_create_fptree();
        }

	    gettimeofday(&t1, NULL);
        ftree = fp_insert_itemset(ftree, curr->itemset, 0);
	    gettimeofday(&t2, NULL);

        elapsedTime += (t2.tv_sec - t1.tv_sec)*1000 + 
              ((t2.tv_usec - t1.tv_usec)/1000.0);

        CNT++;
        stream = curr;
        curr = curr->next;
        fp_delete_data_node(stream->itemset);
        free(stream);
    }
    fclose(fp);
    gettimeofday(&tt2, NULL);

    printf("total time taken to insert in FP tree = %lf ms\n", elapsedTime);

    // pruning last batch
    // gettimeofday(&tp1, NULL);
    // fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
    // process_batch(ptree, CNT/BATCH_SIZE);
    // gettimeofday(&tp2, NULL);

    // pruneTime += (tp2.tv_sec - tp1.tv_sec)*1000 + 
    //   ((tp2.tv_usec - tp1.tv_usec)/1000.0);
=======
    gettimeofday(&t1, NULL);
    gettimeofday(&origin, NULL);
    // gettimeofday(&global_timer, NULL);

    // initializing the first LL of stream separately. Buffer structure is being re-used here.
    buffer stream = NULL, end = NULL;
    stream = (buffer) calloc(1, sizeof(struct buffer_node));
    // stream->itemset = (data) calloc(1, sizeof(struct data_node));
    // stream->itemset->next = NULL;
    stream->next = NULL;
    end = stream;

    while(fscanf(sf, "%d", &sz) != EOF)
    {
        data d = malloc((sz+2)*sizeof(int));
        d[0] = 0;
        d[1] = 0;
        while(sz--)
        {
            data_type item;
            fscanf(sf, "%d", &item);
            d[d[1] + 2] = item;
            d[1]++;
            batch_no++;
        }

        d = sf_sort_data(d); // canonical sort of incoming trans
        // sf_print_data_node(d);
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        fscanf(poisson, "%lf", &delay_time);
        end->freq = delay_time / RATE_PARAMETER; /* this time is in milli-seconds, reusing the freq field to store delay_time*/
        end->next = NULL;
    }

    fclose(sf);
    end = stream;
    stream = stream->next;
    // sf_print_buffer(stream);
    // sf_delete_data_node(end->itemset); 
    // deleting the head of itemset LL which was dummy.
    free(end);
    /* Stream has been read */

    int buffered = 0;
    while(stream)
    {
        gettimeofday(&t3, NULL);
        if(RATE_PARAMETER < 0) /* this means that we have all the time to insert the item */
            sf_insert_itemset(forest, stream->itemset, tid, stream->freq, NULL);
        else
            /* t3 has the time when the insertion started */
            sf_insert_itemset(forest, stream->itemset, tid, stream->freq, &t3);
        
        gettimeofday(&t4, NULL);
        buffered += LEAVE_AS_BUFFER;
        LEAVE_AS_BUFFER = 0;
        // sf_fp_insert(tree->root, tree->head_table, d->next, tid);

        elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
        elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
        insertionTime += elapsedTime;

        end = stream->next;
        sf_delete_data_node(stream->itemset); // freeing the transaction after inserting.
        free(stream);
        stream = end;

        // sf_prune(forest, tid);
        // break;
        /* intermittent pruning */
        if(tid%BATCH == 0)
        {
            // fprintf(stdout, "pruning at tid = %d\n", tid);
            gettimeofday(&t3, NULL);
            // sf_empty_buffers(forest, tid);
            sf_prune(forest, tid);
            gettimeofday(&t4, NULL);
            elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
            elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
            prune_time += elapsedTime;
        }
        tid++;
    }

    
    N = tid;
    /* this is to accomodate hard support counts instead of % */
    if(SUP > 1.0)
    {
        // if(SUP/N - EPS < 0) /* mine with very less support */
            SUP = SUP/N;
        // else
        //     SUP = SUP/N - EPS;
    }
    // else
    // {
    //     if(SUP - EPS > 0)
    //         SUP = SUP - EPS;
    // }

    gettimeofday(&t2, NULL);

    for(i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "MAX_BUFFER_SIZE[%d] = %u\n", i, MAX_BUFFER_SIZE[i]);
    
    fprintf(stdout, "***************************************************\n");
    
    for(i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "MIN_BUFFER_SIZE[%d] = %u\n", i, MIN_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

    for(i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "AVG_BUFFER_SIZE[%d] = %lf\n", i, (float)AVG_BUFFER_SIZE[i]/CNT_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

    for(i = 1; i < LEAVE_LVL + 1; i++)
        fprintf(stdout, "RED_BUFFER_SIZE[%d] = %lf\n", i, (float)RED_BUFFER_SIZE[i]/CNT_BUFFER_SIZE[i]);

    fprintf(stdout, "***************************************************\n");

>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746

    // printf("total time taken to prune the Pattern tree = %lf ms\n", pruneTime);

    pruneTime = (tt2.tv_sec - tt1.tv_sec)*1000 + 
      ((tt2.tv_usec - tt1.tv_usec)/1000.0);
    printf("total time taken to insert+prune the Pattern tree = %lf ms\n", pruneTime);

<<<<<<< HEAD
    // fp = fopen("./tests/final.gnd", "w");
    // fprintf(fp, "After batch %d:\n", CNT/BATCH_SIZE);
    // fclose(fp);

    // fp_delete_fptree(ftree);

    // Final Mining
    printf("Mining with support count = %lf\n", CNT*MINSUP_FREQ);
    gettimeofday(&t1, NULL);
    fp_create_header_table(ftree);
    // ftree = get_fptree(ptree);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, 1);
=======
    fprintf(stdout, "total time taken to insert in sf tree = %lf ms, buffered = %d\n", elapsedTime, buffered);
    fprintf(stdout, "average time to insert in sf tree = %lf ms\n", insertionTime/tid);

    fprintf(stdout, "total intermittent prune time = %lf ms\n", prune_time);
    fprintf(stdout, "avg. intermittent prune time = %lf ms\n", prune_time/(N/BATCH));

    // sf_print_sforest(forest);

    gettimeofday(&t3, NULL);
    sf_prune(forest, tid); // final pruning before emptying the buffers
    sf_empty_buffers(forest, tid, TIME_MINE);
    gettimeofday(&t4, NULL);

    // sf_print_sforest(forest);
    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    fprintf(stdout, "total time taken to empty/prune the buffers = %lf ms\n", elapsedTime);

    gettimeofday(&t1, NULL);
    no_patterns = sf_mine_frequent_itemsets(forest, tid, pattern); // mining for frequent patterns
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
<<<<<<< HEAD

    printf("total time taken to mine pattern tree = %lf ms\n", elapsedTime);

    fp_delete_data_node(sorted);
    return 1;
}
=======
    fprintf(stdout, "(%d freq. patterns) total time taken to mine the sf tree = %lf ms\n",\
            no_patterns, elapsedTime);

    fflush(stdout);
    // to do final free
    // sf_delete_sforest(forest);
    // free(forest);
    // // sf_delete_data_node(sorted);
    return 0;
}
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746
