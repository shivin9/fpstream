#define GLOBAL_VARS 0
#include <mpi.h>
#include "sfstream.h"

/* This code was originally for making SWP-tree an anytime algorithm*/

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100,
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = 3, BUFFER_SIZE = 100, STREAMS = 2;

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

buffer read_stream(FILE* sf, FILE* poisson, int *total_items)
{
    // initializing the first LL of stream separately. Buffer structure is being re-used here.
    int sz, i;
    double delay_time;
    buffer stream = NULL, end = NULL, curr = NULL;
    stream = (buffer) calloc(1, sizeof(struct buffer_node));
    stream->next = NULL;
    end = stream;

    while (fscanf(sf, "%d", &sz) != EOF)
    {
        data d = malloc((sz + 2) * sizeof(int));
        i = 2;
        // printf("sz =  %d\n", sz);
        d[0] = 0;
        d[1] = sz;
        while (sz--)
        {
            data_type item;
            fscanf(sf, "%d", &item);
            // printf("%d, ", item);
            d[i++] = item;
        }
        // sf_print_data_node(d);
        d = sf_sort_data(d); // canonical sort of incoming transactions
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        fscanf(poisson, "%lf", &delay_time);
        end->freq = delay_time / RATE_PARAMETER; /* this time is in milli-seconds, reusing the freq field to store delay_time*/
        (*total_items)++;
        // end->next = NULL;
    }
    end = stream;
    stream = stream->next;
    curr = stream;

    // sf_print_buffer(stream);
    sf_delete_data_node(end->itemset);
    // deleting the head of itemset LL which was dummy.
    free(end);
    printf("Read the Stream...\n");
    /* Stream has been read */
    return curr;
}


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
    // strcpy(OUT_FILE, argv[2]); // the name of the output file is being copied.
    // sf = fopen(OUT_FILE, "w");
    // fclose(sf);
    
    char *s, output;
    
    int i, j, total_items = 0, item_no = 0, sleepTime = 100, tree_to_prune, sz, batch_size, cnt,\
        item_ready, leavecnt = 1, tid = 1, pattern = 0, no_patterns = 0, curr_tree;
    
    int leave_as_buffer;
    pdata sorted = create_sorted_dummy();

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
                case 'n':
                    STREAMS = strtof(s, &s);
                    break;
                default:
                    fprintf(stdout, "UNKNOWN ARGUMENT! %c\n", *(s - 1));
                    exit(-1);
                    break;
                }
            }
        }
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
            <LEAVE_LVL>:        %d\n\
            <STREAMS>           %d\n",
            DICT_SIZE, HSIZE, BATCH, BUFFER_SIZE,
            DECAY, EPS, RATE_PARAMETER,
            CARRY, GAMMA, THETA, TIME_MINE, SUP,
            LEAVE_LVL, STREAMS);
            
    srand(time(NULL));
    poisson = fopen(".poisson.ignore", "r");

    sforest forest[STREAMS];

    for (i = 0; i < STREAMS; i++)
        forest[i] = sf_create_sforest();

    int world_size;

    const int MAX_NUMBERS = 1000000000;
    // buffer items = (buffer) calloc(MAX_NUMBERS, sizeof(struct buffer_node));
    char *items = (char *) calloc(MAX_NUMBERS, sizeof(char));

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size != STREAMS)
    {
        fprintf(stderr, "Must use %d processes for this example\n", STREAMS);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm MPI_SLAVES, MPI_MASTER;

    if(world_rank == 0) // the pattern tree
    {
        MPI_Comm_split(MPI_COMM_WORLD, 0, world_rank, &MPI_MASTER);
        MPI_Status status;
        // items = (buffer) calloc(MAX_NUMBERS, sizeof(struct buffer_node));
        int item_count, itemset_len;
        printf("master node started\n");

        FILE *output;
        do
        {
            output = fopen("output", "a");
            fprintf(output, "After batch %d:\n", batch_ready);
            fclose(output);
            i = 1;
            while(i < world_size)
            {
                printf("receiving items from slave no. %d\n", i);
                /* receiving a string from the slaves */
                MPI_Recv(items, 10000000, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
                /* converting the strings to a buffer array */
                buffer trans = sf_string2buffer(items);
                item_count = trans[0].ftid; /* small hack to store the total number of itemsets */

                printf("master received total %d items from %d, tag = %d\n",
                       item_count, status.MPI_SOURCE, status.MPI_TAG);
                
                for (j = 0; j < item_count; j++)
                {
                    // sf_print_buffer_node(trans[j]);
                    // printf("j = %d\n", j);
                    // print_pdata_node(data2pdata(trans[j].itemset));
                    sf_prefix_inset_itemset(forest[0], trans[j].itemset, trans[j].freq, batch_ready);
                }
                i++;
            }
            printf("inserted all itemsets in the main forest!\n");
            batch_ready++;
            // aux = get_fptree(ptree);
            item_no += item_count;

            printf("mining main with freq = %lf\n", item_no * SUP);
			// sf_print_sforest(forest[0]);
			sf_mine_frequent_itemsets(forest[0], item_no, 2, world_rank);
            // fp_mine_frequent_itemsets(aux, sorted, NULL, item_no, 1);
            // fp_delete_fptree(aux);
        } while (1);
    }

    else if(world_rank > 0) // any tree
    {
        int row_rank, row_size;
        MPI_Comm_split(MPI_COMM_WORLD, 1, world_rank, &MPI_MASTER);

        MPI_Comm_rank(MPI_MASTER, &row_rank);
        MPI_Comm_size(MPI_MASTER, &row_size);
         
        printf("WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE: %d/%d\n",
            world_rank, world_size, row_rank, row_size);
            
        struct timeval t1, t2, t3, t4;
        double elapsedTime, sum = 0, totaltime = 0, prune_time = 0, insertionTime = 0, delay_time;
        gettimeofday(&origin, NULL);
        gettimeofday(&t1, NULL);

        int i, j, total_items, item_no = 0, fetched_items;
        sf = fopen(argv[world_rank], "r"); // input  tells which tree to read from which file
        printf("reading file %s\n", argv[world_rank]);
        if (sf == NULL)
        {
            fprintf(stdout, "invalid file\n");
            exit(0);
        }
        total_items = 0;
        buffer curr = read_stream(sf, poisson, &total_items), stream = NULL;
        // sf_print_buffer(curr);
        do
        {
            if (item_no <= total_items)
            {
                // printf("inserting item no.: %d in TREE_%d... \n", item_no, world_rank);
                // sf_print_data_node(curr->itemset);
                gettimeofday(&t3, NULL);
                sf_insert_itemset(forest[world_rank], curr->itemset, item_no, curr->freq, &t3);
                item_no++;

                if (curr->next)
                {
                    stream = curr;
                    free(curr->itemset);
                    curr = curr->next;
                    free(stream);
                }

                /* prune the tree if 5000 transactions have been inserted or if the stream has ended */
                if(item_no % BATCH == 0 || item_no == total_items)
                {
                    // N = item_no;
                    cnt = sf_mine_frequent_itemsets(forest[world_rank], item_no, -2, world_rank);
                    printf("sender:%d sending %d items\n", world_rank, cnt);
                    char* items = sf_get_trans(world_rank);
                    unsigned long size = strlen(items) + 1;
/* 
                    printf("testing sf_get_trans function which fetched %d items\n", fetched_items);
                    for(i = 0; i < fetched_items; i++)
                    {
                        // sf_print_buffer_node(items[i]);
                        size += sizeof(items[i]);
                        size += sizeof(items[i].itemset);
                        // printf("sizeof(items[%d] = %ld\n", i, sizeof(items[i]));
                        // sf_print_data_node(items[1].itemset);
                    }
                    sf_prune(forest[world_rank], tid);
 */
                    MPI_Barrier(MPI_MASTER);
                    MPI_Send(items, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                    printf("FOREST_%d has sent items\n", world_rank);
                    MPI_Barrier(MPI_MASTER);
                }
            }
            else
            {
                MPI_Finalize();
                break;
            }
        }while (1);
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
/*     else
    {
        if(SUP - EPS > 0)
            SUP = SUP - EPS;
    }

    gettimeofday(&t2, NULL);


    sf_print_sforest(forest);
    elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
    elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
    fprintf(stdout, "total time taken to empty/prune the buffers = %lf ms\n", elapsedTime);


    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    fprintf(stdout, "(%d freq. patterns) total time taken to mine the sf tree = %lf ms\n",
            no_patterns, elapsedTime);
 */
    fflush(stdout);
    // to do final free
    // sf_delete_sforest(forest);
    // free(forest);
    // // sf_delete_data_node(sorted);
    return 0;
}
