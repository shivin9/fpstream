#define GLOBAL_VARS 1
#include "sfstream.h"

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100,\
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3, BUFFER_SIZE = 100,\
    RANK = 1, STREAMS = 0;

/* structures for conducting tests*/
unsigned int MAX_BUFFER_SIZE[10], CNT_BUFFER_SIZE[10],\
             AVG_BUFFER_SIZE[10], MIN_BUFFER_SIZE[10] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},\
             RED_BUFFER_SIZE[10];

long int N;
char OUT_FILE[100];

double DECAY = 1.0, EPS = 0.0, THETA = 0.1, GAMMA=2.0,\
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01,\
       H_FRACTION = 0.1, RATE_PARAMETER = 0.1, CARRY = 1.0,\
       TIME_MINE = 1000.0;

timeval origin, global_timer;

int main(int argc, char* argv[])
{
    if(argc == 1)
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
                -R<RANK>\n\
                -p<pattern>\n");
        exit(-1);
    }

    FILE *sf, *poisson, *state; // files to read the data set and poisson values.
    int sz, cnt, tid = 1, pattern = 0, no_patterns = 0, transactions = 0;
    long pos = 0, size;

    sf = fopen("intermediate", "w");
    fclose(sf);
    sf = fopen("output", "w");
    fclose(sf);


    char* s, output;

    int i;
    for (i = 2; i < argc; i++)
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
                    case 'n': STREAMS = strtof(s, &s);         break;
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
                    case 'R': RANK =  strtof(s, &s);           break;
                    default : fprintf(stdout, "UNKNOWN ARGUMENT! %c", *(s-1));
                              exit(-1);                        break;
                }
            }
        }
    }

    if(pattern != -2) /* -2 means MP-stream */
    {
        strcpy(OUT_FILE, argv[2]); // the name of the output file is being copied.
        sf = fopen(OUT_FILE, "w");
        fclose(sf);
    }

    sf = fopen(argv[1], "r");
    if(sf == NULL)
    {
        fprintf(stdout, "invalid file\n");
        exit(0);
    }
    fseek(sf, 0, SEEK_END);
    size = ftell(sf);
    rewind(sf);

    char file[33];
    sprintf(file, "%d", RANK);
    char *fname = concat(".state_", file);

    state = fopen(fname, "r");
    if (state == NULL)
    {
        state = fopen(fname, "w");
        fprintf(state, "%ld", pos);
    }
    fscanf(state, "%ld", &pos);
    if (pos == -1L)
    {
        printf("file has already been read!\n");
        fclose(state);
        exit(0);
    }
    
    fclose(state);
    state = fopen(fname, "w");

    HSIZE = H_FRACTION*DICT_SIZE; // size of the hash table computed here

    /* fprintf(stdout, "\
            The parameters are:-\n\
            <DICT_SIZE>:        %d\n\
            <HSIZE>:            %d\n\
            <BATCH_SIZE>:       %d\n\
            <BUFFER_SIZE>:      %d\n\
            <DECAY>:            %lf\n\
            <EPS>:              %lf\n\
            <RATE_PARAMETER>:   %lf\n\
            <RANK>:             %d\n\
            <CARRY>             %lf\n\
            <GAMMA>             %lf\n\
            <THETA>:            %lf\n\
            <TIME_MINE>:        %lf\n\
            (S/s)<SUP>:         %lf\n\
            <LEAVE_LVL>:        %d\n",
            DICT_SIZE, HSIZE, BATCH, BUFFER_SIZE,
            DECAY, EPS, RATE_PARAMETER, RANK,
            CARRY, GAMMA, THETA, TIME_MINE, SUP,
            LEAVE_LVL); */

    srand(time(NULL));
    poisson = fopen("poisson.ignore", "r");
    if(poisson == NULL)
    {
        color("RED");
        printf("Please create poisson.ignore!\n");
        reset();
        exit(0);
    }

    // long unsigned size;
    sforest forest = NULL;

    // data sorted = sf_create_sorted_dummy(0);
    forest = sf_create_sforest(); // initializing the forest and creating root nodes of all the trees.

    // sftree tree = sf_create_sftree(0);
    // sf_create_header_table(tree, tid);

    struct timeval t1, t2, t3, t4;
    double elapsedTime, sum = 0, totaltime = 0, prune_time = 0, insertionTime = 0, delay_time;

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
    fseek(sf, pos, SEEK_SET);

    while(transactions < BATCH && (fscanf(sf, "%d", &sz) != EOF))
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
        }

        d = sf_sort_data(d); // canonical sort of incoming trans
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        fscanf(poisson, "%lf", &delay_time);
        end->freq = delay_time / RATE_PARAMETER; /* this time is in milli-seconds, reusing the freq field to store delay_time*/
        end->next = NULL;
        transactions++;
    }
    // printf("transactions = %d\n", transactions);

    if (size == ftell(sf))
        fprintf(state, "%ld", -1L);
    else
        fprintf(state, "%ld", ftell(sf) + 1);
    fclose(state);

    fclose(sf);
    end = stream;
    stream = stream->next;

    // sf_delete_data_node(end->itemset); 

    /* deleting the head of itemset LL which was dummy. */
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
        // if(tid%BATCH == 0)
        // {
        //     // fprintf(stdout, "pruning at tid = %d\n", tid);
        //     gettimeofday(&t3, NULL);
        //     // sf_empty_buffers(forest, tid);
        //     sf_prune(forest, tid);
        //     gettimeofday(&t4, NULL);
        //     elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
        //     elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
        //     prune_time += elapsedTime;
        // }
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

    // for(i = 1; i < LEAVE_LVL + 1; i++)
    //     fprintf(stdout, "MAX_BUFFER_SIZE[%d] = %u\n", i, MAX_BUFFER_SIZE[i]);
    
    // fprintf(stdout, "***************************************************\n");
    
    // for(i = 1; i < LEAVE_LVL + 1; i++)
    //     fprintf(stdout, "MIN_BUFFER_SIZE[%d] = %u\n", i, MIN_BUFFER_SIZE[i]);

    // fprintf(stdout, "***************************************************\n");

    // for(i = 1; i < LEAVE_LVL + 1; i++)
    //     fprintf(stdout, "AVG_BUFFER_SIZE[%d] = %lf\n", i, (float)AVG_BUFFER_SIZE[i]/CNT_BUFFER_SIZE[i]);

    // fprintf(stdout, "***************************************************\n");

    // for(i = 1; i < LEAVE_LVL + 1; i++)
    //     fprintf(stdout, "RED_BUFFER_SIZE[%d] = %lf\n", i, (float)RED_BUFFER_SIZE[i]/CNT_BUFFER_SIZE[i]);

    // fprintf(stdout, "***************************************************\n");


    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    // fprintf(stdout, "total time taken to insert in sf tree = %lf ms, buffered = %d\n", elapsedTime, buffered);
    // fprintf(stdout, "average time to insert in sf tree = %lf ms\n", insertionTime/tid);

    // fprintf(stdout, "total intermittent prune time = %lf ms\n", prune_time);
    // fprintf(stdout, "avg. intermittent prune time = %lf ms\n", prune_time/(N/BATCH));

    // sf_print_sforest(forest);

    gettimeofday(&t3, NULL);
    sf_prune(forest, tid); // final pruning before emptying the buffers
    sf_empty_buffers(forest, tid, TIME_MINE);
    gettimeofday(&t4, NULL);

    // sf_print_sforest(forest);
    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    // fprintf(stdout, "TOTAL TIME TAKEN TO EMPTY/PRUNE THE BUFFERS IN TREE %d= %lf ms\
    //                  AFTER TID = %d\n", RANK, elapsedTime, tid);

    gettimeofday(&t1, NULL);
    no_patterns = sf_mine_frequent_itemsets(forest, tid, -2, RANK); // mining for frequent patterns
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    color("GREEN");
    fprintf(stdout, "(%d freq. patterns) TOTAL TIME TAKEN TO MINE BLTREE %d = %lf ms\n",\
            no_patterns, RANK, elapsedTime);
    reset();

    fflush(stdout);
    // to do final free
    // sf_delete_sforest(forest);
    // free(forest);
    // // sf_delete_data_node(sorted);
    return 1;
}
