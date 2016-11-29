#include "fpstream.h"


int cmpfunc (const void * a, const void * b)
{
   return -( *(int*)a - *(int*)b );
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

    int sz;
    fptree ftree = NULL;

    data sorted = create_sorted_dummy();
    int cnt = 0, sum = 0;
    int batch_size = 10000;
    ftree = fp_create_fptree();

    while(fscanf(fp, "%d", &sz) != EOF){
        /*if(cnt % batch_size == 0){
            if(cnt/batch_size > 0){

                fp_create_header_table(ftree);
                // fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);

                FILE *fp1;
                fp1 = fopen("output", "a");
                fprintf(fp1, "After batch %d:\n", cnt/batch_size);
                fclose(fp1);

                fp_delete_fptree(ftree);
                // ftree = get_fptree(ptree);
                // fp_mine_frequent_itemsets(ftree, sorted, NULL, 1);
            }

            // fp_delete_fptree(ftree);
            ftree = fp_create_fptree();
        }*/

        // printf("sz = %d\n", sz);
        data d = NULL;
        while(sz--){

            data_type item;
            fscanf(fp, "%d", &item);

            data new_d = malloc(sizeof(struct data_node));
            if(new_d == NULL){
                printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
        }
        // removes duplicates items also
        // fp_print_data_node(d);
        fp_sort_data(d, NULL);
        // fp_print_data_node(d);
        ftree = fp_insert_itemset(ftree, d);
        fp_delete_data_node(d);
        cnt++;
    }
    fclose(fp);

    // printf("\n\nfinal pattern tree:\n\n");
    // print_tree(ptree->root);
    // printf("\nresulting fp-tree:\n\n");
    // fp_print_tree(ftree->root);
    printf("sizeof fp tree = %d\n", fp_size_of_tree(ftree->root));
    // printf("\n");

    fp_create_header_table(ftree);
    int* arr = (int*) malloc(DICT_SIZE * sizeof(int));
    int* funcarr = (int*) malloc(DICT_SIZE * sizeof(int));

    fp_sort_header_table(ftree->head_table, funcarr);
    // fp_print_header_table(ftree->head_table);
    // process_batch(ptree, cnt/batch_size);
    // fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);

    for(cnt = 0; cnt < 100; cnt++)
        arr[cnt] = 0;

    cnt = 0;
    fpnode_list child = ftree->root->children;
    while(child){
        // printf("<%d, %d> ", child->tree_node->data_item, child->tree_node->freq);
        // printf("%d ", child->tree_node->freq);
        arr[cnt++] =  child->tree_node->freq;
        sum += arr[cnt-1];
        child = child->next;
    }

    qsort(arr, 100, sizeof(int), cmpfunc);
    printf("total = %d, children = %d\n", sum, cnt);
    for(cnt = 0; cnt < 100; cnt++)
        printf("%d ", arr[cnt]);
    printf("\n");


    double t1 = time(NULL);

    // fp_print_tree(ftree->root);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
    // fp_print_tree(ftree->root);
    double t2 = time(NULL);
    printf("total time taken by FP tree = %lf\n", (t2-t1));

    fptree ctree = fp_convert_to_CP(ftree);
    fp_sort_data(sorted, funcarr);
    sorted = fp_reverse_data(sorted);
    // fp_print_data_node(sorted);
    // fp_print_header_table(ctree->head_table);

    // fp_print_tree(ctree->root);

    // fp_delete_fptree(ftree);
    // fptree ctree = ftree;


    t1 = time(NULL);
    fp_mine_frequent_itemsets(ctree, sorted, NULL, 0);
    t2 = time(NULL);
    printf("total time taken by CP tree = %lf\n", (t2-t1));



    // printf("\nresulting cp-tree:\n");
    // fp_print_tree(ctree->root);
    // printf("\n");
    printf("\nsizeof cp tree = %d\n", fp_size_of_tree(ctree->root));

    child = ctree->root->children;
    for(cnt = 0; cnt < 100; cnt++)
        arr[cnt] = 0;
    cnt = 0, sum = 0;

    while(child){
        // printf("<%d, %d> ", child->tree_node->data_item, child->tree_node->freq);
        // printf("%d ", child->tree_node->freq);
        arr[cnt++] =  child->tree_node->freq;
        sum += arr[cnt-1];
        child = child->next;
    }

    qsort(arr, 100, sizeof(int), cmpfunc);
    printf("total = %d, children = %d\n", sum, cnt);
    for(cnt = 0; cnt < 100; cnt++)
        printf("%d ", arr[cnt]);

    printf("\n\nHeader Table:\n");

    fp_sort_header_table(ctree->head_table, arr);
    fp_delete_fptree(ctree);
    fp_delete_data_node(sorted);
    free(arr);
    return 0;
}