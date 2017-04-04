#include "sfstream.h"
/*
    note: be careful with mining functions of cptree and sforest
    all are working correct but just that they need to be properly adjusted ie. sorting the sorted list for cptree and initializing all the arrays to 0 before using etc.

    CP and sf trees are working alright
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

    FILE *sf;
    int max = 0;
    sf = fopen("intermediate", "w");
    fclose(sf);
    sf = fopen("output", "w");
    fclose(sf);

    sf = fopen(argv[1], "r");

    if(sf == NULL){
        printf("invalid file\n");
        exit(0);
    }
    // sfstream(argv[1]);

    int sz, tid = 1, cnt, batch_size = 10000;
    double size;
    sforest forest = NULL;

    data sorted = sf_create_sorted_dummy(0);
    forest = sf_create_sforest();
    // sf_create_header_table(forest, tid);

    struct timeval t1, t2, t3, t4;
    double elapsedTime, sum = 0, totaltime = 0;

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
        int res = sf_insert_itemset(forest, d, tid);
        gettimeofday(&t4, NULL);

        elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0;
        elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0;
        totaltime += elapsedTime;

        max = max<res ? res:max;
        // sf_create_header_table_helper(forest->root, forest->head_table);
        // sf_update_header_table(forest->head_table, d, tid);
        // sf_print_tree(forest->root);
        sf_delete_data_node(d);
        // sf_prune(forest, tid);
        // break;
        if(tid%1000 == 0)
        {
            // sf_create_header_table_helper(forest->root, forest->head_table);
            // sf_update_header_table(forest->head_table, sorted, tid);
            // sf_print_header_table(forest->head_table);
            size = sf_size_of_sforest(forest)/1000000.0;
            printf("pruning at tid = %d, size = %lf Mb; ", tid, size);
            sf_prune(forest, tid);
            size = sf_size_of_sforest(forest)/1000000.0;
            printf("new_size = %lf Mb\n", size);
            // break;
        }
        tid++;
    }
    fclose(sf);

    /* Create the perfect, final tree after emptying the buffers*/
    // sf_empty_buffers(forest, forest, tid);
    // sf_prune(forest, tid);

    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to insert in sf tree = %lf ms\n", elapsedTime);
    // sf_print_sforest(forest);
    printf("sizeof sf tree = %d\n", sf_size_of_sforest(forest));

    printf("average time to insert in sf tree = %lf ms\n", totaltime/tid);

    gettimeofday(&t1, NULL);
    sf_mine_frequent_itemsets(forest, tid, 0);
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    printf("total time taken to mine the sf tree = %lf ms\n", elapsedTime);
    printf("max queue length = %d\n", max);
    printf("q size = %ld\n", max*sizeof(snode));

    printf("no. of %lf nodes = %lu", 4.0, sf_no_of_nodes(forest[1]->root, 4.0));

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

    sf_delete_sforest(forest);
    free(forest);
    sf_delete_data_node(sorted);
    return 0;
}