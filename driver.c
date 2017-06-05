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

int cmpfunc (const void * a, const void * b)
{
   return -( *(int*)a - *(int*)b );
}

int CNT = 0;

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
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

    fp = fopen(argv[1], "r");

    if(fp == NULL)
    {
        printf("invalid file\n");
        exit(0);
    }
    // fpstream(argv[1]);

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
        if(CNT > 0 && CNT % BATCH_SIZE == 0)
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
    gettimeofday(&tp1, NULL);
    fp_create_header_table(ftree);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
    process_batch(ptree, CNT/BATCH_SIZE);
    gettimeofday(&tp2, NULL);

    pruneTime += (tp2.tv_sec - tp1.tv_sec)*1000 + 
      ((tp2.tv_usec - tp1.tv_usec)/1000.0);

    printf("total time taken to prune the Pattern tree = %lf ms\n", pruneTime);

    pruneTime = (tt2.tv_sec - tt1.tv_sec)*1000 + 
      ((tt2.tv_usec - tt1.tv_usec)/1000.0);
    printf("total time taken to insert+prune the Pattern tree = %lf ms\n", pruneTime);

    // fp = fopen("./tests/final.gnd", "w");
    // fprintf(fp, "After batch %d:\n", CNT/BATCH_SIZE);
    // fclose(fp);

    fp_delete_fptree(ftree);

    // Final Mining
    printf("Mining with support count = %lf\n", CNT*MINSUP_FREQ);
    gettimeofday(&t1, NULL);
    ftree = get_fptree(ptree);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, 1);
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to mine pattern tree = %lf ms\n", elapsedTime);

    fp_delete_data_node(sorted);
    return 1;
}