#define GLOBAL_VARS 1
#include "sftree.h"
/*
    note: be careful with mining functions of cptree and sforest
    all are working correct but just that they need to be properly adjusted ie. sorting the sorted list for cptree and initializing all the arrays to 0 before using etc.

    CP and sf trees are working alright
*/

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100,\
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3, BUFFER_SIZE = 100;

unsigned int MAX_BUFFER_SIZE[10], CNT_BUFFER_SIZE[10],\
             AVG_BUFFER_SIZE[10], MIN_BUFFER_SIZE[10] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},\
             RED_BUFFER_SIZE[10];

long int N;
char OUT_FILE[100];

double DECAY = 1.0, EPS = 0.0, THETA = 0.1,\
       SUP, MINSUP_FREQ = 0.02, MINSUP_SEMIFREQ = 0.01,\
       CARRY = 1.0, H_FRACTION = 0.1, RATE_PARAMETER = 0.1;


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
    int sz, cnt, tid = 1, pattern = 0, no_patterns = 0, file = 0, buffered = 0;
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
                    case 's': SUP    = strtof(s, &s);          break;
                    case 'm': MINSUP_SEMIFREQ = strtof(s, &s); break;
                    case 'M': MINSUP_FREQ = strtof(s, &s);     break;
                    case 'B': BATCH  =       strtod(s, &s);    break;
                    case 'b': BUFFER_SIZE  = strtod(s, &s);    break;
                    case 'p': pattern  =   strtod(s, &s);      break;
                    case 'D': DICT_SIZE =  strtod(s, &s);      break;
                    case 'H': H_FRACTION =  strtof(s, &s);     break;
                    case 'S': SUP =  strtof(s, &s);            break;
                    case 'L': LEAVE_LVL =  strtod(s, &s);      break;
                    case 'r': RATE_PARAMETER =  strtof(s, &s); break;
                    default : fprintf(stdout, "UNKNOWN ARGUMENT! %c", *(s-1));
                              exit(-1);                        break;
                }
            }
        }
    }

    sf = fopen(OUT_FILE, "w");
    fclose(sf);

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
            <THETA>:            %lf\n\
            (S/s)<SUP>:         %lf\n\
            <LEAVE_LVL>:        %d\n",\
            DICT_SIZE, HSIZE, BATCH, BUFFER_SIZE,\
            DECAY, EPS, RATE_PARAMETER,\
            CARRY, THETA, SUP, LEAVE_LVL);

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
    char* files[10]={
                        "./data/10MD500T15_1.data",
                        "./data/10MD500T15_2.data",
                        "./data/10MD500T15_3.data",
                        "./data/10MD500T15_4.data",
                        "./data/10MD500T15_5.data",
                        "./data/10MD500T15_6.data",
                        "./data/10MD500T15_7.data",
                        "./data/10MD500T15_8.data",
                        "./data/10MD500T15_9.data",
                        "./data/10MD500T15_10.data"
                    };

    gettimeofday(&t1, NULL);
    while(file < 10)
    {
        sf = fopen(files[file++], "r");
        if(sf == NULL)
        {
            fprintf(stdout, "invalid file\n");
            exit(0);
        }

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
                //if(sz < 3 || rand()%3 == 0) /* to get down the avg. len*/
                {
                        d[d[1] + 2] = item;
                        d[1]++;
                }
            }
            if(d[1] > 0)
            {
                d = sf_sort_data(d); // canonical sort of incoming trans
                // sf_print_data_node(d);
                end->next = (buffer) calloc(1, sizeof(struct buffer_node));
                end = end->next;
                end->itemset = d;
                fscanf(poisson, "%lf", &delay_time);
                end->freq = delay_time / RATE_PARAMETER; /* this time is in milli-seconds, reusing the freq field to store delay_time*/
                end->next = NULL;
                batch_no++;
            }
        }

        fclose(sf);
        end = stream;
        stream = stream->next;
        // sf_print_buffer(stream);
        // sf_delete_data_node(end->itemset); 
        // deleting the head of itemset LL which was dummy.
        free(end);
        
        while(stream)
        {
            // // fprintf(stdout, "inserting: ");
            // sf_print_data_node(stream->itemset);

            gettimeofday(&t3, NULL);
            if(RATE_PARAMETER < 0)
                sf_insert_itemset(forest, stream->itemset, tid, stream->freq, NULL);
            else
                sf_insert_itemset(forest, stream->itemset, tid, stream->freq, &t3);
            gettimeofday(&t4, NULL);
            
            buffered += LEAVE_AS_BUFFER;
            LEAVE_AS_BUFFER = 0;
            // sf_fp_insert(tree->root, tree->head_table, d->next, tid);

            elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
            elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
            insertionTime += elapsedTime;

            end = stream->next;
            sf_delete_data_node(stream->itemset); // freeing the transaction after inserting. // change it to array.
            free(stream);
            stream = end;

            // sf_prune(forest, tid);
            // break;
            // intermittent pruning
            if(tid%BATCH == 0)
            {
                fprintf(stdout, "pruning at tid = %d\n", tid);
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
    }
    
    N = tid;
    /* this is to accomodate hard support counts instead of %*/
    if(SUP > 1.0)
    {
        // if(SUP/N - EPS < 0) /* mine with very less support*/
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


    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    fprintf(stdout, "total time taken to insert in sf tree = %lf ms, buffered = %d\n", elapsedTime, buffered);
    fprintf(stdout, "average time to insert in sf tree = %lf ms\n", insertionTime/tid);

    fprintf(stdout, "total intermittent prune time = %lf ms\n", prune_time);
    fprintf(stdout, "avg. intermittent prune time = %lf ms\n", prune_time/(N/BATCH));

    // sf_print_sforest(forest);

    gettimeofday(&t3, NULL);
    sf_prune(forest, tid); // final pruning before emptying the buffers
    sf_empty_buffers(forest, tid, 1000.0);
    gettimeofday(&t4, NULL);

    // sf_print_sforest(forest);
    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    fprintf(stdout, "total time taken to empty/prune the buffers = %lf ms\n", elapsedTime);

    gettimeofday(&t1, NULL);
    no_patterns = sf_mine_frequent_itemsets(forest, tid, pattern); // mining for frequent patterns
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    fprintf(stdout, "(%d freq. patterns) total time taken to mine the sf tree = %lf ms\n",\
            no_patterns, elapsedTime);

    fflush(stdout);
    // to do final free
    // sf_delete_sforest(forest);
    // free(forest);
    // // sf_delete_data_node(sorted);
    return 0;
}
