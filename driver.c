#include "sfstream.h"
/*
    note: be careful with mining functions of cptree and sftree
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

    int sz, tid = 1, size, cnt, batch_size = 10000;
    sftree ftree = NULL;

    data sorted = sf_create_sorted_dummy();
    ftree = sf_create_sftree();
    sf_create_header_table(ftree, tid);

    struct timeval t1, t2;
    double elapsedTime, sum = 0;

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
        // removes duplicates items also
        // printf("inserting: ");
        sf_sort_data(d, NULL);
        // sf_print_data_node(d);
        ftree = sf_insert_itemset(ftree, d, tid, 0);
        sf_create_header_table_helper(ftree->root, ftree->head_table);
        sf_update_header_table(ftree->head_table, d, tid);
        // sf_print_tree(ftree->root);
        sf_delete_data_node(d);
        sf_prune(ftree, tid);
        // break;
        if(tid%1000 == 0)
        {
<<<<<<< HEAD
            fp_create_header_table_helper(ftree->root, ftree->head_table);
            fp_update_header_table(ftree->head_table, sorted, tid);
            // fp_print_header_table(ftree->head_table);
            size = fp_size_of_tree(ftree->root);
=======
            sf_create_header_table_helper(ftree->root, ftree->head_table);
            sf_update_header_table(ftree->head_table, sorted, tid);
            // sf_print_header_table(ftree->head_table);
            size = sf_size_of_tree(ftree->root);
>>>>>>> 664578536f97317d67b70abe10652d79c66ebff8
            printf("pruning at tid = %d, size = %d; ", tid, size);
            sf_prune(ftree, tid);
            size = sf_size_of_tree(ftree->root);
            printf("new_size = %d\n", size);
            // break;
        }
        tid++;
    }
    fclose(sf);
    /* Create the perfect, final tree after emptying the buffers*/
<<<<<<< HEAD
    fp_create_header_table_helper(ftree->root, ftree->head_table);
    fp_empty_buffers(ftree->root, ftree->head_table, tid);
    fp_update_header_table(ftree->head_table, sorted, tid);
    fp_prune(ftree, tid);
=======
    sf_create_header_table_helper(ftree->root, ftree->head_table);
    sf_empty_buffers(ftree->root, ftree->head_table, tid);
    sf_update_header_table(ftree->head_table, sorted, tid);
    sf_prune(ftree, tid);
>>>>>>> 664578536f97317d67b70abe10652d79c66ebff8

    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to insert in sf tree = %lf ms\n", elapsedTime);

    // printf("\n\nfinal pattern tree:\n\n");
    // print_tree(ptree->root);
    // printf("\nresulting sf-tree:\n\n");
    // printf("\n");
    // sf_print_tree(ftree->root);

    double* arr = (double*) calloc(DICT_SIZE, sizeof(double));
    double* funcarr = (double*) calloc(DICT_SIZE, sizeof(double));

    // sf_print_header_table(ftree->head_table);
    // process_batch(ptree, cnt/batch_size);
    // sf_mine_frequent_itemsets(ftree, sorted, NULL, 0);
    int cnt, idx;
    for(cnt = 0; cnt < 100; cnt++){
        arr[cnt] = 0.0;
        funcarr[cnt] = 0.0;
    }
    // sf_create_header_table(ftree);
    // usleep(1000);
<<<<<<< HEAD
    // fp_create_header_table_helper(ftree->root, &(ftree->head_table));
    fp_sort_header_table(ftree->head_table, funcarr);
    // fp_print_header_table(ftree->head_table);

    printf("\nMINSUP_FREQ = %lf, MINSUP_SEMIFREQ = %lf, SUP = %lf\n\n", MINSUP_FREQ, MINSUP_SEMIFREQ, SUP);
    printf("total time taken to insert in FP tree = %lf ms\n", elapsedTime);
=======
    sf_empty_buffers(ftree->root);
    // sf_create_header_table_helper(ftree->root, &(ftree->head_table));
    // sf_sort_header_table(ftree->head_table, funcarr);
    // sf_print_header_table(ftree->head_table);
>>>>>>> 664578536f97317d67b70abe10652d79c66ebff8

    cnt = 0;
    sfnode* child = ftree->root->children;

    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        if(child[idx])
        {
            // printf("<%d, %d> ", child->tree_node->data_item, child->tree_node->freq);
            // printf("%lf ", child->tree_node->freq);
            arr[cnt] =  child[idx]->freq;
            sum += arr[cnt++];
        }
    }

    printf("sizeof sf tree = %d\n", sf_size_of_tree(ftree->root));

    qsort(arr, 100, sizeof(double), cmpfunc);

    // printf("total = %d, children = %d\n", sum, cnt);
    // for(cnt = 0; cnt < 100; cnt++)
    //     printf("%lf ", arr[cnt]);
    // printf("\n");

    // correct sf tree 437 new500
    gettimeofday(&t1, NULL);
    sf_mine_frequent_itemsets(ftree, sorted, NULL, tid, 0);
    /* to print the summary of root node*/
    printf("total = %lf, children = %d\n", sum, cnt);
    for(cnt = 0; cnt < 100; cnt++)
        printf("%lf ", arr[cnt]);
    printf("\n");

    // correct sf tree 437 new500
    gettimeofday(&t1, NULL);
<<<<<<< HEAD
    fp_mine_frequent_itemsets(ftree, sorted, NULL, NULL, tid, 2);
=======
    sf_mine_frequent_itemsets(ftree, sorted, NULL, NULL, tid, 2);
>>>>>>> 664578536f97317d67b70abe10652d79c66ebff8
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to mine sf tree = %lf ms\n", elapsedTime);
    // sf_print_tree(ftree->root);

    sftree ctree = ftree;
    // sf_empty_buffers(ftree);

    sf_sort_data(sorted, funcarr);
    sorted = sf_reverse_data(sorted);
    gettimeofday(&t1, NULL);
<<<<<<< HEAD
    // fp_mine_frequent_itemsets(ctree, sorted, NULL, NULL, tid, 2);
=======
    // sf_mine_frequent_itemsets(ctree, sorted, NULL, NULL, tid, 2);
>>>>>>> 664578536f97317d67b70abe10652d79c66ebff8
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken by CP tree = %lf ms\n", elapsedTime);

    // printf("\nresulting cp-tree:\n");
    // sf_print_tree(ctree->root);
    // printf("\n");
    printf("\nsizeof cp tree = %d\n", sf_size_of_tree(ctree->root));

    child = ctree->root->children;

    for(cnt = 0; cnt < 100; cnt++)
        arr[cnt] = 0.0;

    cnt = 0, sum = 0;

    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        if(child[idx])
        {
            // printf("<%d, %d> ", child->tree_node->data_item, child->tree_node->freq);
            // printf("%d ", child->tree_node->freq);
            arr[cnt++] =  child[idx]->freq;
            sum += arr[cnt-1];
        }
    }
    qsort(arr, 100, sizeof(double), cmpfunc);

    /* to print the summary of root node*/
    // printf("total = %lf, children = %d\n", sum, cnt);
    // for(cnt = 0; cnt < 100; cnt++)
    //     printf("%lf ", arr[cnt]);

    sf_delete_sftree(ctree);
    sf_delete_data_node(sorted);
    free(arr);
    free(funcarr);
    return 1;
}