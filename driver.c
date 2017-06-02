#include "fpstream.h"
/*
    note: be careful with mining functions of cptree and fptree
    all are working correct but just that they need to be properly adjusted ie. sorting the sorted list for cptree and initializing all the arrays to 0 before using etc.

    CP and FP trees are working alright
*/
// call this function to start a nanosecond-resolution timer
// struct timespec timer_start(){
//     struct timespec start_time;
//     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
//     return start_time;
// }

// // call this function to end a timer, returning nanoseconds elapsed as a long
// long timer_end(struct timespec start_time){
//     struct timespec end_time;
//     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
//     long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
//     return diffInNanos;
// }

/*DESCENDING order here*/
int cmpfunc (const void * a, const void * b)
{
   return -( *(double*)a - *(double*)b );
}

int main(int argc, char* argv[])
{
    if(argc == 1){
        printf("Please enter filename!\n");
        exit(-1);
    }

    FILE *fp;

    fp = fopen("intermediate", "w");
    fclose(fp);
    fp = fopen("output", "w");
    fclose(fp);

    fp = fopen(argv[1], "r");

    if(fp == NULL){
        printf("invalid file\n");
        exit(0);
    }
    // fpstream(argv[1]);

    int sz, tid = 1, size, cnt, batch_size = 10000;
    fptree ftree = NULL;

    data sorted = fp_create_sorted_dummy();
    ftree = fp_create_fptree();
    fp_create_header_table(ftree, tid);

    struct timeval t1, t2, tp1, tp2, tt1, tt2;
    double elapsedTime, sum = 0, mineTime = 0, pruneTime = 0, totalTime = 0;

    buffer stream = NULL, end = NULL, curr = NULL;
    stream =  (buffer) calloc(1, sizeof(struct buffer_node));
    stream->itemset = (data) calloc(1, sizeof(struct data_node));
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

            data new_d = calloc(1, sizeof(struct data_node));
            if(new_d == NULL)
            {
                printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
            // sleepTime = 2000;
            // usleep(sleepTime);
        }
        fp_sort_data(d, NULL);
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        end->next = NULL;
    }
    end->next = NULL;
    curr = curr->next;
    printf("read stream\n");

    gettimeofday(&tt1, NULL);
    while(curr)
    {
        // printf("inserting: ");
        // fp_print_data_node(curr->itemset);
        gettimeofday(&t1, NULL);
        ftree = fp_insert_itemset(ftree, curr->itemset, tid, 0);
        gettimeofday(&t2, NULL);
        elapsedTime += (t2.tv_sec - t1.tv_sec) * 1000.0;
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

        stream = curr;
        fp_delete_data_node(curr->itemset);
        free(stream);
        curr = curr->next;

        if(tid % BATCH_SIZE == 0)
        {
            gettimeofday(&t1, NULL);
            fp_create_header_table_helper(ftree->root, ftree->head_table);
            fp_update_header_table(ftree->head_table, sorted, tid);
            // fp_print_header_table(ftree->head_table);
            fp_prune(ftree, tid);
            gettimeofday(&t2, NULL);
            printf("pruning at tid = %d\n", tid);

            pruneTime += (t2.tv_sec - t1.tv_sec) * 1000.0;
            pruneTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
        }
        tid++;
    }
    fclose(fp);

    gettimeofday(&t1, NULL);
    /* Create the perfect, final tree after emptying the buffers*/
    fp_create_header_table_helper(ftree->root, ftree->head_table);
    fp_empty_buffers(ftree->root, ftree->head_table, tid);
    fp_update_header_table(ftree->head_table, sorted, tid);
    fp_prune(ftree, tid);
    gettimeofday(&t2, NULL);
    gettimeofday(&tt2, NULL);

    totalTime = (tt2.tv_sec - tt1.tv_sec) * 1000.0;
    totalTime += (tt2.tv_usec - tt1.tv_usec) / 1000.0;
    printf("Total time taken to insert+prune in FP tree = %lf ms\n", totalTime);

    pruneTime += (t2.tv_sec - t1.tv_sec) * 1000.0;
    pruneTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("MINSUP_FREQ = %lf, MINSUP_SEMIFREQ = %lf, SUP = %lf\n\n", MINSUP_FREQ, MINSUP_SEMIFREQ, SUP);
    printf("total time taken to insert in FP tree = %lf ms\n", elapsedTime);
    printf("total time taken to prune FP tree = %lf ms\n", pruneTime);

    // correct fp tree 437 new500
    gettimeofday(&t1, NULL);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, NULL, tid, 1);
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to mine FP tree = %lf ms\n", elapsedTime);

    fp_delete_data_node(sorted);
    return 1;
}