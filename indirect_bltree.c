#define GLOBAL_VARS 0
#include <mpi.h>
#include "sfstream.h"
#include "str_builder.h"

/* 
    Let forest be the tree in which all streams are getting merged.
    After they are taken cared of, we move on to merging the tilted time
    windows.

    Master tree is writing result in result_0. It uses result_-1 as auxillary file.
*/

int BATCH = 1000, DICT_SIZE = 100, HSIZE = 100,
    LEAVE_AS_BUFFER = 0, LEAVE_LVL = INT_MAX, BUFFER_SIZE = 100, STREAMS = 2;

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

char *build_command(int argc, char *argv[], int rank)
{
    str_builder_t *sb;
    char *str;
    sb = str_builder_create();
    str_builder_add_str(sb, "./slave ", 0);

    str_builder_add_str(sb, argv[rank], 0);
    str_builder_add_str(sb, " ", 0);

    int i;
    for (i = STREAMS; i < argc; i++)
    {
        str_builder_add_str(sb, argv[i], 0);
        str_builder_add_str(sb, " ", 0);
    }
    str_builder_add_str(sb, "-R", 0);
    str_builder_add_int(sb, rank);
    return (char *)str_builder_peek(sb);
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
    sf = fopen("result_0", "w");
    fclose(sf);
    
    
    // sfstream(argv[1]);
    // strcpy(OUT_FILE, argv[2]); // the name of the output file is being copied.
    // sf = fopen(OUT_FILE, "w");
    // fclose(sf);
    
    char *s, output;
    
    int i, j, total_items = 0, item_no = 0, sleepTime = 100, tree_to_prune, sz, batch_size, cnt,\
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

    LEAVE_LVL = DICT_SIZE;

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

    sforest forest;
    forest = sf_create_sforest();

    struct sf_TT_wndw tt_window[64];

    for(i = 0; i < 64; i++)
    {
        // tt_window[i].main = sf_create_sforest();
        // tt_window[i].temp = sf_create_sforest();        
        tt_window[i].main = NULL;
        tt_window[i].temp = NULL;
    }


    int world_size;

    const int MAX_NUMBERS = INT_MAX;
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

    if (world_rank == 0) // the pattern tree
    {
        MPI_Comm_split(MPI_COMM_WORLD, 0, world_rank, &MPI_MASTER);
        MPI_Status status;
        // items = (buffer) calloc(MAX_NUMBERS, sizeof(struct buffer_node));
        int item_count, itemset_len, total, exit_count = 0;
        printf("master node started\n");
        color("BLUE");
        FILE *output;
        do
        {
            output = fopen("output", "a");
            fprintf(output, "After batch %d:\n", batch_ready);
            fclose(output);
            i = 1;
            total = 0;
            /* as processes exit, we need to adjust the counter */
            while(i < world_size) 
            {
                item_count = 0;
                // printf("receiving items from slave no. %d\n", i);

                /* receiving a string from the slaves */
                MPI_Recv(items, 10000000, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
                // printf("%s\n", items);
                if (!strcmp(items, "fin"))
                {
                    printf("received fin from slave %d\n", i);
                    exit_count++;
                }
                else
                {
                    /* converting the strings to a buffer array */
                    buffer trans = sf_string2buffer(items);

                    item_count = trans[0].ftid; /* small hack to store the total number of itemsets */
                    total += item_count;
                    
                    printf("master received %d items from %d, tag = %d, batch = %d\n",
                        item_count, status.MPI_SOURCE, status.MPI_TAG, batch_ready);

                    color("GREEN");
                    for (j = 0; j < item_count; j++)
                    {
                        // sf_print_buffer_node(trans[j]);
                        // data d = sf_sort_data(trans[j].itemset); // canonical sort of incoming trans
                        sf_prefix_insert_itemset(forest, trans[j].itemset, trans[j].freq, batch_ready);
                    }
                    // MPI_Send("processed", 10, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                    color("RED");
                    printf("Inserted %d itemsets in the main forest from slave %d!\n", item_count, i);
                }
                i++;
            }
            // sf_peel_tree(forest, -1);

            // printf("exit_cnt = %d, world_size = %d\n", exit_count, world_size);
            if (exit_count == (world_size - 1))
            {
                color("RED");
                printf("\nALL SLAVES HAVE ENDED, MASTER QUITING. HAVE A GOOD DAY!\n");
                reset();
                MPI_Finalize();

                color("RED");
                printf("MASTER IS DONE...\n");
                reset();

                sf = fopen("result_0", "w");
                fclose(sf);

                for(i = 0; i < 64; i++)
                {
                    printf("mining tt-window %d\n", i);
                    sf = fopen("result_0", "a");
                    fprintf(sf, "\nResult of TT-window %d\n", i);
                    fclose(sf);

                    sf_mine_frequent_itemsets(tt_window[i].main, item_no, 2, world_rank);
                }

                break;
            }

            batch_ready++;
            /* this means that no itemsets were sent to master */
            if (item_count == 0)
                break;

            color("MAGENTA");
            printf("Inserted total %d itemsets in the main forest in batch = %d!\n", total, batch_ready);
            reset();
            // aux = get_fptree(ptree);
            item_no += total;

            /* mine the tree when needed. pattern = 2 => mine with SUP */
            printf("MINING MAIN TREE WITH FREQ = %lf\n\n", item_no * SUP);
			// int mined_cnt = sf_mine_frequent_itemsets(forest, item_no, 2, world_rank);
            sf_update_TTW(tt_window, forest);
            // sf_print_TTW(tt_window);

            color("YELLOW");
            // printf("\n+++\nMINED %d ITEMS FROM TREE 0 IN BATCH %d\n+++\n", mined_cnt, batch_ready);
            reset();

            /* initialize a new forest after every batch */
            // sf_delete_sforest(forest[0]);
            // sforest temp = forest;
            forest = sf_create_sforest();
            // sf_delete_sforest(temp);
            // free(temp);
            reset();
        } while (1);
        
        // color("RED");
        // printf("MASTER IS DONE...\n");
        // reset();

        // sf = fopen("result_0", "w");
        // fclose(sf);

        // for(i = 0; i < 64; i++)
        // {
        //     printf("mining tt-window %d\n", i);
        //     sf = fopen("result_0", "a");
        //     fprintf(sf, "\nResult of TT-window %d\n", i);
        //     fclose(sf);

        //     sf_mine_frequent_itemsets(tt_window[i].main, item_no, 2, world_rank);
        // }
    }

    else if (world_rank > 0) // any tree
    {
        long pos = 0;
        int row_rank, row_size, child_parent_pipe[2], parent_child_pipe[2];
        
        MPI_Comm_split(MPI_COMM_WORLD, 1, world_rank, &MPI_MASTER);

        MPI_Comm_rank(MPI_MASTER, &row_rank);
        MPI_Comm_size(MPI_MASTER, &row_size);
         
        printf("WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE: %d/%d\n",
            world_rank, world_size, row_rank, row_size);

        reset();
        char* cmd = build_command(argc, argv, world_rank);
        color("RED");
        printf("cmd for process %d = %s", world_rank, cmd);
        reset();

        int i, j, total_items, item_no = 0, fetched_items;
        printf("\n+++\nMINING ITEMS FROM SLAVE TREE %d with freq = %lf\n+++\n", world_rank, item_no*EPS);
        // sf = fopen(argv[world_rank], "r"); // input tells which tree to read from which file

        char state_file[33];
        sprintf(state_file, "%d", world_rank);
        char *fname = concat(".state_", state_file);
        FILE *state = fopen(fname, "r");

        pipe(child_parent_pipe);
        pipe(parent_child_pipe);
        pid_t pid = fork();
        /* execute the child process till the time all batches have not been consumed */
        if (pid == 0)
        {
            close(child_parent_pipe[0]); /* close the reading end */
            // close(parent_child_pipe[1]); /* close the writing end */
            int child_status = 0, parent_status = 1;

            if (state == NULL)
            {
                state = fopen(fname, "w");
                fprintf(state, "%ld", pos);
            }

            fscanf(state, "%ld", &pos);
            fclose(state);
            // pos = 0;
            color("MAGENTA");
            printf("pos = %ld, state = %s\n", pos, fname);
            reset();
            int btch = 0;

            /* execute the child process till the time all batches have not been consumed */
            while (pos != -1L)
            {
                // while(parent_status != 1) /* wait for parent to get ready */
                // {
                //     read(parent_child_pipe[0], &parent_status, sizeof(parent_status));
                // }
                child_status = -1;
                write(child_parent_pipe[1], &child_status, sizeof(child_status));
                system(cmd);

                child_status = 1;
                /* communicate status to parent */
                write(child_parent_pipe[1], &child_status, sizeof(child_status)); 

                state = fopen(fname, "r");
                fscanf(state, "%ld", &pos);
                fclose(state);
                // printf("pos = %ld, mined batch = %d\n", pos, btch++);
            }
            printf("\n(+)(+)(+) ALL BATCHES HAVE BEEN MINED!! (+)(+)(+)\n");
            child_status = 2;
            write(child_parent_pipe[1], &child_status, sizeof(child_status));
        }
        else
        {
            int child_status, parent_status, exit_status = 0;
            close(child_parent_pipe[1]); /* close the writing end */
            // close(parent_child_pipe[0]); /* close the reading end */
            // fscanf(state, "%ld", &pos);
            while(1) /* execute the parent process till state != -1 */
            {
                read(child_parent_pipe[0], &child_status, sizeof(child_status));
                while(child_status != 1)
                {
                    read(child_parent_pipe[0], &child_status, sizeof(child_status));
                    if (child_status == 2)
                    {
                        color("GREEN");
                        printf("(*)(*)(*) ALL BATCHES READ IN SLAVE %d (*)(*)(*)\n", world_rank);
                        printf("slave %d sent a FIN signal\n", world_rank);
                        MPI_Send("fin", 4, MPI_CHAR, 0, world_rank, MPI_COMM_WORLD); /* send the FIs in form of string */
                        // while(1);
                        reset();
                        exit_status = 1;
                        break;
                    }

                parent_status = 0; /* I'm busy now! */
                // write(parent_child_pipe[1], &parent_status, sizeof(parent_status));

                printf("BATCH MINING COMPLETED IN SLAVE%d!\n", world_rank);
                char* items = sf_get_trans(world_rank); /* read the mined transactions in string form */
                unsigned long size = strlen(items) + 1;

                // printf("testing sf_get_trans function which fetched %d items\n", fetched_items);
                // sf_delete_sforest(forest[world_rank]);
                // forest[world_rank] = sf_create_sforest();

                MPI_Barrier(MPI_MASTER);
                MPI_Send(items, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD); /* send the FIs in form of string */
                MPI_Barrier(MPI_MASTER);
                // MPI_Recv(signal_from_master, 10000000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
                // if (strcmp(signal_from_master, "processed"))
                // {
                //     printf("MASTER finished with processing the data I(%d) sent\n", world_rank);
                //     parent_status = 1;
                //     signal_from_master = "";
                // }

                parent_status = 1; /* I'm free now! */
                // write(parent_child_pipe[1], &parent_status, sizeof(parent_status));
                reset();
            }
        }
    }
    fflush(stdout);
    // to do final free
    // sf_delete_sforest(forest);
    // free(forest);
    // // sf_delete_data_node(sorted);
    return 0;
}
