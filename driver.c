#include "sfstream.h"
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
<<<<<<< HEAD
<<<<<<< HEAD
    fp_create_header_table(ftree, 1);

    struct timeval t1, t2;
    double elapsedTime;
=======
=======
>>>>>>> new
    fp_create_header_table(ftree, tid);

    struct timeval t1, t2;
    double elapsedTime, sum = 0;
<<<<<<< HEAD
>>>>>>> new
=======
>>>>>>> new

    gettimeofday(&t1, NULL);

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
                printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
        }
        // removes duplicates items also
        // printf("inserting: ");
        fp_sort_data(d, NULL);
        // fp_print_data_node(d);
        ftree = fp_insert_itemset(ftree, d, tid, 0);
<<<<<<< HEAD
<<<<<<< HEAD
        fp_create_header_table_helper(ftree->root, ftree->head_table);
        fp_update_header_table(ftree->head_table, d, tid);
        // fp_print_tree(ftree->root);
        fp_delete_data_node(d);
        fp_prune(ftree, tid);
=======


        // fp_print_header_table(ftree->head_table);
        fp_delete_data_node(d);
=======


        // fp_print_header_table(ftree->head_table);
        fp_delete_data_node(d);
>>>>>>> new
        // break;
        if(tid%1000 == 0)
        {
            fp_create_header_table_helper(ftree->root, ftree->head_table);
            fp_update_header_table(ftree->head_table, sorted, tid);
            // fp_print_header_table(ftree->head_table);
            size = fp_size_of_tree(ftree->root);
            printf("pruning at tid = %d, size = %d; ", tid, size);
            fp_prune(ftree, tid);
            size = fp_size_of_tree(ftree->root);
            printf("new_size = %d\n", size);
            // break;
        }
<<<<<<< HEAD
>>>>>>> new
=======
>>>>>>> new
        tid++;
    }
    fclose(fp);

<<<<<<< HEAD
<<<<<<< HEAD
=======
=======
>>>>>>> new

    /* Create the perfect, final tree after emptying the buffers*/
    fp_create_header_table_helper(ftree->root, ftree->head_table);
    fp_empty_buffers(ftree->root, ftree->head_table, tid);
    fp_update_header_table(ftree->head_table, sorted, tid);
    fp_prune(ftree, tid);

<<<<<<< HEAD
>>>>>>> new
=======
>>>>>>> new
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

<<<<<<< HEAD
<<<<<<< HEAD
    printf("total time taken to insert in FP tree = %lf ms\n", elapsedTime);

    // printf("\n\nfinal pattern tree:\n\n");
    // print_tree(ptree->root);
=======

>>>>>>> new
=======

>>>>>>> new
    // printf("\nresulting fp-tree:\n\n");
    // printf("\n");
    // fp_print_tree(ftree->root);

    double* arr = (double*) calloc(DICT_SIZE, sizeof(double));
    double* funcarr = (double*) calloc(DICT_SIZE, sizeof(double));

    // fp_print_header_table(ftree->head_table);
<<<<<<< HEAD
<<<<<<< HEAD
    // process_batch(ptree, cnt/batch_size);
    // fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
    int cnt, idx;
    for(cnt = 0; cnt < 100; cnt++){
        arr[cnt] = 0.0;
        funcarr[cnt] = 0.0;
    }
    // fp_create_header_table(ftree);
    // usleep(1000);
    fp_empty_buffers(ftree->root);
    // fp_create_header_table_helper(ftree->root, &(ftree->head_table));
    // fp_sort_header_table(ftree->head_table, funcarr);
    // fp_print_header_table(ftree->head_table);

    cnt = 0;
    fpnode* child = ftree->root->children;

    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        if(child[idx])
        {
            // printf("<%d, %d> ", child->tree_node->data_item, child->tree_node->freq);
            // printf("%lf ", child->tree_node->freq);
            arr[cnt] =  child[idx]->freq;
            sum += arr[cnt++];
        }
=======

    // usleep(1000);
=======

    // usleep(1000);
>>>>>>> new
    // fp_create_header_table_helper(ftree->root, &(ftree->head_table));
    fp_sort_header_table(ftree->head_table, funcarr);
    // fp_print_header_table(ftree->head_table);

    printf("\nMINSUP_FREQ = %lf, MINSUP_SEMIFREQ = %lf, SUP = %lf\n\n", MINSUP_FREQ, MINSUP_SEMIFREQ, SUP);
    printf("total time taken to insert in FP tree = %lf ms\n", elapsedTime);

    cnt = 0;
    fpnode_list child = ftree->root->children;
    while(child){
        // printf("<%d, %d> ", child->tree_node->data_item, child->tree_node->freq);
        // printf("%lf ", child->tree_node->freq);
        arr[cnt] =  child->tree_node->freq;
        sum += arr[cnt++];
        child = child->next;
>>>>>>> new
    }

    printf("sizeof fp tree = %d\n", fp_size_of_tree(ftree->root));

    qsort(arr, 100, sizeof(double), cmpfunc);
<<<<<<< HEAD
<<<<<<< HEAD

    // printf("total = %d, children = %d\n", sum, cnt);
    // for(cnt = 0; cnt < 100; cnt++)
    //     printf("%lf ", arr[cnt]);
    // printf("\n");

    // correct fp tree 437 new500
    gettimeofday(&t1, NULL);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, tid, 0);
=======

=======

>>>>>>> new
    /* to print the summary of root node*/
    printf("total = %lf, children = %d\n", sum, cnt);
    for(cnt = 0; cnt < 100; cnt++)
        printf("%lf ", arr[cnt]);
    printf("\n");

    // correct fp tree 437 new500
    gettimeofday(&t1, NULL);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, NULL, tid, 2);
<<<<<<< HEAD
>>>>>>> new
=======
>>>>>>> new
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken to mine FP tree = %lf ms\n", elapsedTime);
    // fp_print_tree(ftree->root);

    fptree ctree = ftree;
    // fp_empty_buffers(ftree);

    fp_sort_data(sorted, funcarr);
    sorted = fp_reverse_data(sorted);
    gettimeofday(&t1, NULL);
    // fp_mine_frequent_itemsets(ctree, sorted, NULL, NULL, tid, 2);
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken by CP tree = %lf ms\n", elapsedTime);

    // printf("\nresulting cp-tree:\n");
    // fp_print_tree(ctree->root);
    // printf("\n");
    printf("\nsizeof cp tree = %d\n", fp_size_of_tree(ctree->root));

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

    fp_delete_fptree(ctree);
    fp_delete_data_node(sorted);
    free(arr);
    free(funcarr);
    return 1;
}