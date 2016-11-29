#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "fptree.h"
#include "pattern_tree.h"
#include "def.h"


void print_itemset(data d){

    if(d == NULL){
        printf("\n");
        return;
    }
    printf("%d ", d->data_item);
    print_itemset(d->next);
}


data create_sorted_dummy(){
    data d = NULL;
    int next = 0;
    while(next < 5){
        data new_d = malloc(sizeof(struct data_node));
        new_d->data_item = next++;
        new_d->next = d;
        d = new_d;
    }
    return d;
}

void delete_itemset(data d){
    if(d == NULL)    return;
    data temp = d;
    delete_itemset(d->next);
    free(temp);
}


//the driver for the FP-stream data structure, takes input batch-by-batch
//for each batch, it creates an FP-tree, mines the frequent itemsets
//and inserts in the pattern tree
void fpstream(){

    FILE *fp1;
    fp1 = fopen("input.txt", "r");

    int sz;
    // patterntree tree = NULL;
    // tree = create_pattern_tree();

    fptree tree = fp_create_fptree();
    printf("init tree\n");
    // fp_print_tree(tree->root);
    // printf("\n");


    data sorted = create_sorted_dummy();
    int cnt = 0;
    int batch_size = 10;
    while(fscanf(fp1, "%d", &sz) != EOF){

        if(cnt % batch_size == 0){

            // if(cnt/batch_size > 0){
            //     tail_prune(tree->root);
            //     fptree temp_fptree = get_fptree(tree);
            //     mine_frequent_itemsets(temp_fptree);
            //     fp_delete_fptree(temp_fptree);
            // }

            FILE *fp;
            fp = fopen("output", "a");
            fprintf(fp, "\nBatch number %d:\n", cnt/batch_size+1);
            fclose(fp);
        }

        // printf("%d: ", sz);
        data d = NULL;
        while(sz--){

            data_type item;
            fscanf(fp1, "%d", &item);

            data new_d = malloc(sizeof(struct data_node));
            if(new_d == NULL){
                printf("new_d malloc failed\n");
            }
            new_d->data_item = item;
            new_d->next = d;
            d = new_d;
        }

        // tree = insert_itemset(tree, d, cnt/batch_size+1);
        tree = fp_insert_itemset(tree, d);
        cnt++;

    // print_tree(tree->root);
    // tail_prune(tree->root);
    // printf("\nafter tail_prune\n\n");
    // print_tree(tree->root);
    // if(cnt == 1)    break;
    }

// tail_prune(tree->root);
// printf("\n\n\n");
// print_tree(tree->root);

fp_create_header_table(tree);
fp_print_tree(tree->root);
fp_mine_frequent_itemsets(tree, sorted, NULL);
            FILE *fp;
            fp = fopen("intermediate", "w");
            fclose(fp);

// printf("\n\n\n");
// printf("and now the fptree:\n");
// fptree f = get_fptree(tree);
// fp_print_tree(f->root);

    fclose(fp1);

    // create_header_table(tree);
    // mine_frequent_itemsets(tree, sorted, NULL);
    // tree = NULL;

    // printf("final fp-tree:\n");
    // print_tree(tree->root);
    // printf("\n");
    // printf("header table:\n");
    // print_header_table(tree->head_table);


    // printf("sorted: ");
    // print_itemset(sorted);

    // fptree cond_tree = create_conditional_fp_tree(tree, 4);
    // create_header_table(cond_tree);
    // printf("\n");
    // printf("conditional fp-tree:\n");
    // print_tree(cond_tree->root);
    // printf("\n");


    // fptree cond_tree2 = create_conditional_fp_tree(cond_tree, 3);
    // if(cond_tree2 == NULL)    printf("NULL tree");
    // else{
    //     create_header_table(cond_tree2);
    //     printf("\n");
    //     printf("conditional fp-tree:\n");
    //     printf("\n");
    //     print_tree(cond_tree2->root);
    // }
    // printf("\n");

    // cond_tree = create_conditional_fp_tree(cond_tree, 2);
    // if(cond_tree == NULL)    printf("NULL tree");
    // else{
    //     create_header_table(cond_tree);
    //     printf("\n");
    //     printf("conditional fp-tree:\n");
    //     print_tree(cond_tree->root);
    //     printf("\n");
    //     printf("header table:\n");
    //     print_header_table(cond_tree->head_table);
    // }
    // printf("\n");
}




