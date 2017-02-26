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

    int batch_no = 0, stream_batch = 0, sleepTime = 100, tree_to_prune, sz, batch_size, cnt, item_ready, i, leavecnt = 1, tid = 1;

    if(argc == 3)
        sleepTime = atoi(argv[2]);

    else if(argc == 4)
        leavecnt = atoi(argv[3]);

    omp_set_num_threads(3);

    curr_tree = 0;
    T1 = 0;
    T2 = 0;
    extern int leave_as_buffer;
    batch_ready = 0;
    tree_to_prune =-1;
    data curr_itemset;
    fptree ftree = NULL, temp = NULL, aux = NULL;

    buffer stream = NULL, end = NULL, curr = NULL;
    stream =  (buffer) calloc(1, sizeof(struct buffer_node));
    stream->itemset = (data) calloc(1, sizeof(struct data_node));
    stream->itemset->next = NULL;
    stream->next = NULL;
    curr = stream;
    end = stream;

    double* arr = (double*) calloc(DICT_SIZE, sizeof(double));
    data sorted = fp_create_sorted_dummy();

    ftree = fp_create_fptree();
    fp_create_header_table(ftree, tid);
    aux = NULL;

    FILE *fp;
    fp = fopen("intermediate", "w");
    fclose(fp);
    fp = fopen("output", "w");
    fclose(fp);

    /* First read the whole stream and then release the transactions as and when needed*/
    fp = fopen(argv[1], "r");

    if(fp == NULL)
    {
        printf("File does not exist!\n");
        exit(0);
    }

    batch_size = 1;

    while(fscanf(fp, "%d", &sz) != EOF)
    {
        data d = NULL;
        item_ready = 0;
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

        batch_no++;
        fp_sort_data(d, NULL);
        end->next = (buffer) calloc(1, sizeof(struct buffer_node));
        end = end->next;
        end->itemset = d;
        end->next = NULL;
        leave_as_buffer = 1;
        item_ready = 1;
    }

    item_ready = -1;
    printf("Read the Stream...\n");

    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        if(threadId == 0) // master thread
        {
            // 1. Reads the data and pumps it into the stream with poisson distribution.
            // 2. After reading one transcation is complete, it simply notifies the current worker thread to pre-empt from it's work process the newly read transaction.
            // 3. After the worker thread has taken away the transaction, it reads next transaction from the file and pumps it again with poisson distribution.

            cnt = 1;
            // while(1)
            // {
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
            // }
            printf("Released All Items\n");
            item_ready = -1;
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
                // printf("stream_batch = %d, batch_no = %d\n", stream_batch, batch_no);
                if(stream_batch >= batch_no && T2 == 0)
                    break;

                while(item_ready == 0 && stream_batch < cnt)
                {
                    // printf("stuck in T1\n");
                }
                if(stream_batch < cnt && curr_tree == 0 && T1== 0)
                {
                    // usleep(sleepTime*10);
                    // printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 1, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    // printf("leave_as_buffer before = %d\n", leave_as_buffer);
                    assert(ftree->head_table != NULL);
                    ftree = fp_insert_itemset(ftree, curr->itemset, tid++, leave_as_buffer);
                    // printf("leave_as_buffer after = %d\n", leave_as_buffer);
                    stream_batch++;

                    if(curr->next){
                        stream = curr;
                        fp_delete_data_node(curr->itemset);
                        curr = curr->next;
                        free(stream);
                    }

                    // prune the tree if 5000 transactions have been inserted or if the stream has ended
                    if(T2 == 0 && (stream_batch%5000 == 0 || stream_batch >= batch_no))
                    {
                        // direct away the stream
                        printf("\nCHANGING to AUX Tree; SIZE = %d; count = %d\n\n",
                                fp_size_of_tree(ftree->root), stream_batch);
                        aux = fp_create_fptree();
                        fp_create_header_table(aux, tid);
                        // exit(0);
                        # pragma omp critical
                        {
                            curr_tree = 1;
                            T1 = -1;
                            // fp_print_tree(ftree->root);
                            // exit(0);
                            // will implement this function later
                            // ftree = fp_convert_to_CP(ftree);
                            fp_create_header_table_helper(ftree->root, ftree->head_table);
                            fp_empty_buffers(ftree->root, ftree->head_table, tid);
                            fp_update_header_table(ftree->head_table, sorted, tid);
                            fp_prune(ftree, tid);
                            printf("***finished pruning ftree***\n");
                            T1 = 0;
                        }
                    }
                }
            }while(1);
        }

        else if(threadId == 2)
        {
            do{
                // printf("stream_batch = %d, batch_no = %d\n", stream_batch, batch_no);
                if(stream_batch >= batch_no && T2 == 0)
                    break;

                while(item_ready == 0 && stream_batch < cnt)
                {
                    // printf("stuck in T2\n");
                }
                if(stream_batch < cnt && curr_tree == 1  && T2 == 0)
                {
                    // printf("inserting item no.: %d in TREE_%d... T1 = %d, T2 = %d\n", stream_batch, 2, T1, T2);
                    // fp_print_data_node(curr->itemset);
                    aux = fp_insert_itemset(aux, curr->itemset, tid++, leave_as_buffer);

                    stream_batch++;
                    if(curr->next)
                    {
                        stream = curr;
                        fp_delete_data_node(curr->itemset);
                        curr = curr->next;
                        free(stream);
                    }

                    // if main tree is ready then we'll mine aux and append the frequent itemsets in the stream
                    if(T1 == 0)
                    {
                        printf("\nCHANGING to MAIN TREE; SIZE = %d; count = %d\n\n",
                                fp_size_of_tree(aux->root), stream_batch);
                        // direct away the stream
                        # pragma omp critical
                        {
                            curr_tree = 0;
                            T2 = -1;
                            int fixtid = tid;
                            printf("Starting to mine the AUX TREE\n");
                            fp_create_header_table_helper(aux->root, aux->head_table);
                            fp_update_header_table(aux->head_table, sorted, fixtid);
                            fp_empty_buffers(aux->root, aux->head_table, fixtid);
                            batch_no += fp_mine_frequent_itemsets(aux, sorted, NULL, &end, tid, 2);
                            // assert(end->next == NULL);
                            fp_delete_fptree(aux);
                            // aux = fp_convert_to_CP(aux);
                            printf("\nFinished Mining the AUX TREE\n\n");
                            // exit(0);
                            T2 = 0;
                        }
                    }
                }
            }while(1);
        }
        // else if (threadId == 3) // thread handling pattern tree along with
        // {
            // 1. When we receive a notification from one of the worker threads, insert the extracted FIs into the pattern tree.
            // do{
            //     if(T1==1 || T2==1){
            //         tree_to_prune = 1 - curr_tree;
            //         // printf("Servicing TREE_%d, batch_ready=%d\n", tree_to_prune + 1, batch_ready);

            //         if(tree_to_prune){
            //             T2 = -1;
            //             aux = aux;
            //         }
            //         else{
            //             T1 = -1;
            //             aux = ftree;
            //         }

            //         // fp_print_tree(aux->root);
            //         fp_create_header_table(aux);
            //         fp_sort_header_table(aux->head_table, arr);
            //         fp_sort_data(sorted, arr);
            //         sorted = fp_reverse_data(sorted);

            //         # pragma omp critical
            //         {
            //             fp_mine_frequent_itemsets(aux, sorted, NULL, 0);
            //             process_batch(ptree, ++batch_ready);
            //             // printf("Done Servicing TREE_%d\n\n", tree_to_prune + 1);

            //             if(tree_to_prune)
            //             {
            //                 // printf("deleting tree2\n");
            //                 // fp_delete_fptree(aux);
            //                 aux = fp_create_fptree();
            //                 T2 = 0;
            //             }
            //             else{
            //                 // printf("deleting tree1\n");
            //                 // fp_delete_fptree(ftree);
            //                 ftree = fp_create_fptree();
            //                 T1 = 0;
            //             }

            //         }

            //         FILE *fp1;
            //         fp1 = fopen("output", "a");
            //         fprintf(fp1, "After batch %d:\n",batch_ready);
            //         fclose(fp1);

            //         fp_delete_fptree(aux);
            //         aux = get_fptree(ptree);
            //         fp_mine_frequent_itemsets(aux, sorted, NULL, 1);
            //         fp_delete_fptree(aux);
            //     }
            // }while(stream_batch <= batch_no);
        // }
        # pragma omp barrier
        # pragma omp join
    }

    /* Now AUX is not needed anymore*/
    // /* Empty the auxillary tree to get frequent itemsets */
    // if(aux->root)
    //     printf("aux is still there! with size = %d\n", fp_size_of_tree(aux));

    // fp_create_header_table(aux, tid);
    // fp_update_header_table(aux->head_table, sorted, tid);
    // batch_no += fp_mine_frequent_itemsets(aux, sorted, NULL, curr, tid, 0);
    // fp_delete_fptree(aux);

    // while(curr->next)
    // {
    //     printf("LATE INSERT item no.: %d in MAIN Tree... T1 = %d, T2 = %d\n", stream_batch, T1, T2);
    //     stream_batch++;
    //     ftree = fp_insert_itemset(ftree, curr->itemset, tid++, 0);
    //     curr = curr->next;
    //     fp_delete_data_node(curr->itemset);
    //     free(stream);
    // }

    // printf("Tree_%d is still left with %d transactions; SIZE = %d\n", tree_to_prune+1, cnt, fp_size_of_tree(aux->root));

    fp_create_header_table_helper(ftree->root, ftree->head_table);
    fp_update_header_table(ftree->head_table, sorted, tid);
    fp_empty_buffers(ftree->root, ftree->head_table, tid);

    fp_mine_frequent_itemsets(ftree, sorted, NULL, NULL, tid, 0);
    printf("Done with final extraction!\n");

    fp_delete_fptree(ftree);

    // printf("\nresulting fp-tree1:\n\n");
    // fp_print_tree(ftree->root);

    // fp_create_header_table(temp);
    // fp_mine_frequent_itemsets(ftree, sorted, NULL, 0);
    // printf("\nresulting fp-tree2:\n\n");
    // fp_print_tree(ftree->root);
    // fp_print_header_table(temp->head_table);

    // fp_print_tree(temp->root);
    fp_delete_data_node(sorted);
    free(arr);
    free(fp);
    gettimeofday(&t2, NULL);

    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

    printf("total time taken = %lf ms\n", elapsedTime);
    return 1;
}
