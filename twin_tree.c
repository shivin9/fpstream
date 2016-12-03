#include "fpstream.h"

int main(int argc, char* argv[])
{
    if(argc == 1){
        printf("Please enter filename!\n");
        exit(-1);
    }

    omp_set_num_threads(4);
    int batch_no = 0, stream_batch = 0, sleepTime, tree_to_prune, item_ready = 0;

    curr_tree = 0;
    T1 = 0;
    T2 = 0;
    leave_as_buffer = 0;
    batch_ready = 0;
    tree_to_prune = -1;
    data curr_itemset;
    fptree ftree1 = NULL;
    fptree ftree2 = NULL;
    fptree temp = NULL;

    buffer stream = NULL, end = NULL, curr = NULL;
    stream =  (buffer) malloc(sizeof(struct buffer_node));
    stream->itemset = (data) malloc(sizeof(struct data_node));
    stream->itemset->next = NULL;
    stream->next = NULL;
    curr = stream;
    end = stream;

    int* arr = (int*) malloc(DICT_SIZE * sizeof(int));

    data sorted = create_sorted_dummy();

    fp_create_header_table(ftree1);
    fp_create_header_table(ftree2);

    ftree1 = fp_create_fptree();
    ftree2 = fp_create_fptree();

    patterntree ptree = NULL;
    ptree = create_pattern_tree();

    FILE *fp;

    fp = fopen("intermediate", "w");
    fclose(fp);
    fp = fopen("output", "w");
    fclose(fp);

#pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        if(threadId == 0) // master thread
        {
            // 1. Reads the data and pumps it into the stream with poisson distribution.
            // 2. After reading one transcation is complete, it simply notifies the current worker thread to pre-empt from its work
            //    process the newly read transaction.
            // 3. After the worker thread has taken away the transaction, it reads next transaction from the file and pumps it again
            //    with poisson distribution.
            fp = fopen(argv[1], "r");
            if(fp == NULL){
                printf("File does not exist!\n");
                exit(0);
            }

            int sz;
            fptree ftree = NULL;
            int cnt = 0, batch_size = 1;

            while(fscanf(fp, "%d", &sz) != EOF)
            {
                data d = NULL;
                item_ready = 0;
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
                    sleepTime = 100;
                    usleep(sleepTime);
                }

                // # pragma omp critical
                {
                    batch_no++;
                    // printf("\nNEW ITEM: Batch: %d in TREE_%d ... ", batch_no, curr_tree+1);
                    fp_sort_data(d, NULL);
                    end->next = (buffer) malloc(sizeof(struct buffer_node));
                    end = end->next;
                    end->itemset = d;
                    end->next = NULL;
                    // fp_print_data_node(end->itemset);
                    leave_as_buffer = 1;
                    item_ready = 1;
                }
            }
            item_ready = -1;
            end = NULL;
            printf("stream has ended... item_ready = %d\n", item_ready);
        }

        else if (threadId == 1) // worker thread 1 on AnyFPStream
        {
            // 1. Waits in an infinite loop until a notification comes from the master thread for a new transaction.
            // 2. Once notification comes, it tries to insert into the CP-Tree
            // 3. If in between the insertions, a notification of new transaction comes, it pre-emtps and processes the new transaction.
            // 4. While descending the CP-tree it takes care of carrying forward to the next level accumulated buffers.
            // 5. After the stream has switched, you process the tree and notify the thread with ID 3.
            // 6. All other aspects of handling the CP-tree. (Yet to complete)
            do{
                while(item_ready == 0)
                {
                    // printf("stuck in T1\n");
                }
                if(curr_tree == 0 && T1==0)
                {
                    // usleep(sleepTime*10);
                    printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 1, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    ftree1 = fp_insert_itemset(ftree1, curr->itemset, 0);
                    stream_batch++;
                    if(curr->next)
                        curr = curr->next;

                    if(T2==0 && fp_size_of_tree(ftree1->root) > SIZE_LMT)
                    {
                        // direct away the stream
                        printf("\nCHANGING TREE 1->2; SIZE = %d; count = %d\n", fp_size_of_tree(ftree1->root), stream_batch);
                        # pragma omp critical
                        {
                            curr_tree = 1;
                            T1 = -1;
                        }
                        fp_empty_buffers(ftree1->root);
                        ftree1 = fp_convert_to_CP(ftree1);
                        printf("finished converting TREE_1\n");
                        T1 = 1;
                    }
                }
            }while(stream_batch <= batch_no);
        }

        else if(threadId == 2)
        {
            do{
                while(item_ready == 0)
                {
                    // printf("stuck in T2\n");
                }
                if(curr_tree == 1  && T2==0)
                {
                    printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 2, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    ftree2 = fp_insert_itemset(ftree2, curr->itemset, 0);
                    stream_batch++;
                    if(curr->next)
                        curr = curr->next;

                    if(T1==0 && fp_size_of_tree(ftree2->root) > SIZE_LMT)
                    {
                        printf("\nCHANGING TREE 2->1; SIZE = %d; count = %d\n", fp_size_of_tree(ftree2->root), stream_batch);
                        // direct away the stream
                        # pragma omp critical
                        {
                            curr_tree = 0;
                            T2 = -1;
                        }
                        fp_empty_buffers(ftree2->root);
                        ftree2 = fp_convert_to_CP(ftree2);
                        printf("finished converting TREE_2\n");
                        T2 = 1;
                    }
                }
            }while(stream_batch <= batch_no);
        }
        else if (threadId == 3) // thread handling pattern tree along with
        {
            // 1. When we receive a notification from one of the worker threads, insert the extracted FIs into the pattern tree.
            do{
                if(T1==1 || T2==1){
                    tree_to_prune = 1 - curr_tree;
                    printf("Servicing TREE_%d, batch_ready=%d\n", tree_to_prune + 1, batch_ready);

                    if(tree_to_prune){
                        T2 = -1;
                        temp = ftree2;
                    }
                    else{
                        T1 = -1;
                        temp = ftree1;
                    }

                    // fp_print_tree(temp->root);
                    fp_create_header_table(temp);
                    fp_sort_header_table(temp->head_table, arr);
                    fp_sort_data(sorted, arr);
                    sorted = fp_reverse_data(sorted);

                    # pragma omp critical
                    {
                        fp_mine_frequent_itemsets(temp, sorted, NULL, 0);
                        process_batch(ptree, ++batch_ready);
                    }

                    FILE *fp1;
                    fp1 = fopen("output", "a");
                    fprintf(fp1, "After batch %d:\n",batch_ready);
                    fclose(fp1);

                    // fp_delete_fptree(temp);
                    temp = get_fptree(ptree);
                    fp_mine_frequent_itemsets(temp, sorted, NULL, 1);

                    if(tree_to_prune)
                    {
                        // printf("deleting tree2\n");
                        fp_delete_fptree(ftree2);
                        ftree2 = fp_create_fptree();
                        T2 = 0;
                    }
                    else{
                        // printf("deleting tree1\n");
                        fp_delete_fptree(ftree1);
                        ftree1 = fp_create_fptree();
                        T1 = 0;
                    }
                    printf("\nDone Servicing TREE_%d\n", tree_to_prune + 1);
                }
            }while(stream_batch <= batch_no);
        }
    # pragma omp barrier
    }
    // now mine the frequent itemsets from the Pattern Tree (at the end)

    if(tree_to_prune == -1){
        if(curr_tree){
            temp = ftree2;
            tree_to_prune = 1;
        }
        else{
            temp = ftree1;
            tree_to_prune = 0;
        }
    }

    else{
        if(curr_tree && T1 >= 0)
           temp = ftree1;
        else if(!curr_tree && T2 >= 0)
           temp = ftree2;
    }

    int cnt = 0;
    while(curr->next){
        // printf("LATE INSERT item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, tree_to_prune, T1, T2);
        cnt++;
        stream_batch++;
        temp = fp_insert_itemset(temp, curr->itemset, 0);
        curr = curr->next;
    }

    printf("Tree_%d is still left with %d transactions; SIZE = %d\n", tree_to_prune+1, cnt, fp_size_of_tree(temp->root));

    fp_empty_buffers(temp->root);
    temp = fp_convert_to_CP(temp);
    // fp_print_tree(temp->root);

    fp_create_header_table(temp);
    fp_sort_header_table(temp->head_table, arr);
    fp_sort_data(sorted, arr);
    sorted = fp_reverse_data(sorted);

    fp_mine_frequent_itemsets(temp, sorted, NULL, 0);
    process_batch(ptree, ++batch_ready);

    FILE *fp1;
    fp1 = fopen("output", "a");
    fprintf(fp1, "After batch %d:\n",batch_ready);
    fclose(fp1);

    fp_delete_fptree(temp);
    temp = get_fptree(ptree);
    fp_mine_frequent_itemsets(temp, sorted, NULL, 1);

    // printf("\nresulting fp-tree1:\n\n");
    // fp_print_tree(ftree1->root);
    // fp_print_header_table(ftree1->head_table);

    // fp_create_header_table(ftree2);
    // fp_mine_frequent_itemsets(ftree1, sorted, NULL, 0);
    // printf("\nresulting fp-tree2:\n\n");
    // fp_print_tree(ftree1->root);
    // fp_print_header_table(ftree2->head_table);

    // fp_print_tree(temp->root);
    fp_mine_frequent_itemsets(temp, sorted, NULL, 0);
    while(stream){
        curr = stream;
        stream = stream->next;
        fp_delete_data_node(curr->itemset);
        free(curr);
    }
    return 1;
}
