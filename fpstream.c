#include "fpstream.h"

void print_itemset(data d)
{

    if(d == NULL)
    {
        printf("\n");
        return;
    }
    printf("%d ", d->data_item);
    print_itemset(d->next);
}

data create_sorted_dummy()
{
    data d = NULL;
    int next = 0;
    while(next < DICT_SIZE)
    {
        data new_d = (data) malloc(sizeof(struct data_node));
        new_d->data_item = next++;
        new_d->next = d;
        d = new_d;
    }
    return d;
}

void delete_itemset(data d)
{
    if(d == NULL)    return;
    data temp = d;
    delete_itemset(d->next);
    free(temp);
}

void process_batch(patterntree tree, int batch_num)
{

    FILE *fp;
    fp = fopen("intermediate", "r");
    int sz, cnt = 0;
    while(fscanf(fp, "%d", &sz) != EOF)
    {

        int i, t=0, arr[100], val, freq;
        while(sz--)
        {
            fscanf(fp, "%d", &val);
            arr[t++] = val;
        }
        fscanf(fp, "%d", &freq);

        data d = NULL;
        for(i=t-1;i>=0;i--)
        {

            data new_d = malloc(sizeof(struct data_node));
            if(new_d == NULL)
            {
                printf("new_d malloc failed\n");
            }
            new_d->data_item = arr[i];
            new_d->next = d;
            d = new_d;
        }
        // printf("inserting in ptree\n");
        // fp_print_data_node(d);
        insert_itemset(tree, d, batch_num, freq);
        fp_delete_data_node(d);
        cnt++;
    }

    tail_prune(tree->root);
    // reset the file 'intermediate'
    fclose(fp);
    fp = fopen("intermediate", "w");
    fclose(fp);
}


//the driver for the FP-stream data structure, takes input batch-by-batch
//for each batch, it creates an FP-tree, mines the frequent itemsets
//and inserts in the pattern tree
void fpstream(char* fname)
{

    FILE *fp;

    fp = fopen("intermediate", "w");
    fclose(fp);
    fp = fopen("output", "w");
    fclose(fp);

    fp = fopen(fname, "r");

    int sz, i, tempvar;

    patterntree ptree = NULL;
    ptree = create_pattern_tree();
    fptree ftree = NULL;
    ftree = fp_create_fptree();

    data sorted = create_sorted_dummy();
    int cnt = 0;
    int batch_size = 1000;

    //  initial insertion is different
    while(fscanf(fp, "%d", &sz) != EOF)
    {
        if(cnt % batch_size == 0)
        {
            if(cnt/batch_size > 0)
            {
                fp_create_header_table(ftree);
                // ftree = fp_convert_to_CP(ftree);
                fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
                process_batch(ptree, cnt/batch_size);

                FILE *fp1;
                fp1 = fopen("output", "a");
                fprintf(fp1, "After batch %d:\n", cnt/batch_size);
                fclose(fp1);

                fp_delete_fptree(ftree);
                ftree = get_fptree(ptree);
                fp_mine_frequent_itemsets(ftree, sorted, NULL, 1);
            }

            fp_delete_fptree(ftree);
            ftree = fp_create_fptree();
        }

        // printf("%d: ", sz);
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
        fp_sort_data(d, NULL);
        fp_print_data_node(d);
        ftree = fp_insert_itemset(ftree, d, 0);
        cnt++;
    }
    fclose(fp);
    fp_create_header_table(ftree);

    double* arr = (double*) malloc(DICT_SIZE * sizeof(double));
    fp_sort_header_table(ftree->head_table, arr);
    fp_sort_data(sorted, arr);
    sorted = fp_reverse_data(sorted);

    ftree = fp_convert_to_CP(ftree);
    fp_print_tree(ftree->root);
    fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);

    // process_batch(ptree, cnt/batch_size);
    // printf("\n\nfinal pattern tree:\n\n");
    // print_tree(ptree->root);
    // printf("\nand resulting fp-tree:\n\n");
    // fp_print_tree(get_fptree(ptree)->root);
    // printf("\n");


    fp = fopen("output", "a");
    fprintf(fp, "After batch %d:\n", cnt/batch_size);
    fclose(fp);

    // fp_delete_fptree(ftree);
    // ftree = get_fptree(ptree);
    // fp_print_tree(ftree->root);
    // fp_mine_frequent_itemsets(ftree, sorted, NULL, 1);
}