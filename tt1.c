#include "fpstream.h"

int main(int argc, char* argv[])
{
    if(argc == 1){
        printf("format is ./exe <filename><sleepTime(in microseconds)><batchSize>\n");
        exit(-1);
    }

    struct timeval t1, t2;
    double elapsedTime;
    gettimeofday(&t1, NULL);

    int batch_no = 0, stream_batch = 0, sleepTime = 100, tree_to_prune, sz, batch_size, cnt, item_ready, i, leavecnt = 1;

    if(argc == 3)
        sleepTime = atoi(argv[2]);

    else if(argc == 4)
        leavecnt = atoi(argv[3]);

    omp_set_num_threads(4);

    curr_tree = 0;
    T1 = 0;
    T2 = 0;
    extern int leave_as_buffer;
    batch_ready = 0;
    tree_to_prune =-1;
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

    double* arr = (double*) malloc(DICT_SIZE * sizeof(double));

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

    {
        fp = fopen(argv[1], "r");

        if(fp == NULL){
            printf("File does not exist!\n");
            exit(0);
        }

        int sz;
        fptree ftree = NULL;
        batch_size = 1;

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
                // sleepTime = 2000;
                // usleep(sleepTime);
            }

            batch_no++;
            fp_sort_data(d, NULL);
            end->next = (buffer) malloc(sizeof(struct buffer_node));
            end = end->next;
            end->itemset = d;
            end->next = NULL;
            leave_as_buffer = 1;
            item_ready = 1;
        }
        item_ready = -1;
        end = NULL;
        printf("Read the Stream...\n");
    }


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
            cnt = 1;
            while(cnt <= batch_no)
            {
                // # pragma omp critical
                // {
                    item_ready = 1;
                    // printf("Releasing Item no. %d\n", cnt);
                    leave_as_buffer = 0;
                    // sleepTime = rand()%100;
                    cnt += leavecnt;
                    usleep(sleepTime);
                    leave_as_buffer = 1;
                    item_ready = 0;
                // }
            }
            printf("Released All Items\n");
            item_ready = -1;
            end = NULL;
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
                while(item_ready == 0 && stream_batch < cnt)
                {
                    // printf("stuck in T1\n");
                }
                if(stream_batch < cnt && curr_tree == 0 && T1==0)
                {
                    // usleep(sleepTime*10);
                    // printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 1, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    // printf("leave_as_buffer before = %d\n", leave_as_buffer);
                    ftree1 = fp_insert_itemset(ftree1, curr->itemset, 1);
                    // printf("leave_as_buffer after = %d\n", leave_as_buffer);
                    stream_batch++;
                    if(curr->next){
                        stream = curr;
                        fp_delete_data_node(curr->itemset);
                        curr = curr->next;
                        free(stream);
                    }

                    if(T2==0 && fp_size_of_tree(ftree1->root) > SIZE_LMT)
                    {
                        // direct away the stream
                        printf("\nCHANGING TREE 1->2; SIZE = %d; count = %d\n\n", fp_size_of_tree(ftree1->root), stream_batch);
                        # pragma omp critical
                        {
                            curr_tree = 1;
                            T1 = -1;
                        // fp_print_tree(ftree1->root);
                        fp_empty_buffers(ftree1->root);
                        // exit(0);
                        ftree1 = fp_convert_to_CP(ftree1);
                        // printf("finished converting TREE_1\n");
                        T1 = 1;
                        }
                    }
                }
            }while(stream_batch <= batch_no);
        }

        else if(threadId == 2)
        {
            do{
                while(item_ready == 0 && stream_batch < cnt)
                {
                    // printf("stuck in T2\n");
                }
                if(stream_batch < cnt && curr_tree == 1  && T2==0)
                {
                    // printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 2, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    ftree2 = fp_insert_itemset(ftree2, curr->itemset, 1);

                    stream_batch++;
                    if(curr->next){
                        stream = curr;
                        fp_delete_data_node(curr->itemset);
                        curr = curr->next;
                        free(stream);
                    }

                    if(T1==0 && fp_size_of_tree(ftree2->root) > SIZE_LMT)
                    {
                        // printf("\nCHANGING TREE 2->1; SIZE = %d; count = %d\n\n", fp_size_of_tree(ftree2->root), stream_batch);
                        // direct away the stream
                        # pragma omp critical
                        {
                            curr_tree = 0;
                            T2 = -1;
                        fp_empty_buffers(ftree2->root);
                        ftree2 = fp_convert_to_CP(ftree2);
                        // printf("finished converting TREE_2\n");
                        T2 = 1;
                        }
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
                    // printf("Servicing TREE_%d, batch_ready=%d\n", tree_to_prune + 1, batch_ready);

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
                        // printf("Done Servicing TREE_%d\n\n", tree_to_prune + 1);

                        if(tree_to_prune)
                        {
                            // printf("deleting tree2\n");
                            // fp_delete_fptree(ftree2);
                            ftree2 = fp_create_fptree();
                            T2 = 0;
                        }
                        else{
                            // printf("deleting tree1\n");
                            // fp_delete_fptree(ftree1);
                            ftree1 = fp_create_fptree();
                            T1 = 0;
                        }

                    }

                    FILE *fp1;
                    fp1 = fopen("output", "a");
                    fprintf(fp1, "After batch %d:\n",batch_ready);
                    fclose(fp1);

                    fp_delete_fptree(temp);
                    temp = get_fptree(ptree);
                    fp_mine_frequent_itemsets(temp, sorted, NULL, 1);
                    fp_delete_fptree(temp);
                }
            }while(stream_batch <= batch_no);
        }
        # pragma omp barrier
        # pragma omp join
    }

    while(curr->next){
        printf("LATE INSERT item no.: %d in TREE_1... T1 = %d, T2 = %d\n", stream_batch, T1, T2);
        stream_batch++;
        ftree1 = fp_insert_itemset(ftree1, curr->itemset, 0);
        curr = curr->next;
        fp_delete_data_node(curr->itemset);
        free(stream);
    }

    // printf("Tree_%d is still left with %d transactions; SIZE = %d\n", tree_to_prune+1, cnt, fp_size_of_tree(temp->root));

    // fp_empty_buffers(temp->root);
    for(i = 0; i < 2; i++){
        if(i)
            temp = ftree1;
        else
            temp = ftree2;

        fp_empty_buffers(temp->root);
        temp = fp_convert_to_CP(temp);
        fp_sort_header_table(temp->head_table, arr);
        fp_sort_data(sorted, arr);
        sorted = fp_reverse_data(sorted);

        fp_mine_frequent_itemsets(temp, sorted, NULL, 0);
        process_batch(ptree, ++batch_ready);
        printf("Done with leftover Tree_%d!\n", i);

        FILE *fp1;
        fp1 = fopen("output", "a");
        fprintf(fp1, "After batch %d:\n",batch_ready);
        fclose(fp1);
        // temp = get_fptree(ptree);
        // fp_mine_frequent_itemsets(temp, sorted, NULL, 1);
        fp_delete_fptree(temp);
    }

    temp = get_fptree(ptree);
    fp_mine_frequent_itemsets(temp, sorted, NULL, 1);
    delete_pattern_tree(ptree);


    // printf("\nresulting fp-tree1:\n\n");
    // fp_print_tree(ftree1->root);

    // fp_create_header_table(ftree2);
    // fp_mine_frequent_itemsets(ftree1, sorted, NULL, 0);
    // printf("\nresulting fp-tree2:\n\n");
    // fp_print_tree(ftree1->root);
    // fp_print_header_table(ftree2->head_table);

    // fp_print_tree(temp->root);
    fp_mine_frequent_itemsets(temp, sorted, NULL, 0);

    fp_delete_fptree(temp);
    fp_delete_data_node(sorted);

    // free(ftree1);
    // free(ftree2);
    free(arr);
    free(fp);
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken = %lf ms\n", elapsedTime);
    return 1;
}
