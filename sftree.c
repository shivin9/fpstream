/*
1. When deleting a child of a node, fix the child_list as well as the item_list
2. When removing nodes, also fix the header table
3. Header table is now not being recreated from scratch, it is being updated periodically
   using the sf_update_header_table function
4. But the tree is being constructed ie. next_similar and prev_similar pointers are being
   given periodically just before pruning
5. Decide whether a node can have both a buffer fptree along with prev. opened transactions
   or only a fptree
6. Header tables are ignorant of the data item present in the node
7. Have -1 as the root node surely
8. FPTrees have child as linked list but header table is still an array
9. The header table is updated just before pruning the tree
*/
#include "sftree.h"

int sf_is_equal(data d1, data d2)
{
    word_addr_t i;
    for(i = 0; i < d1->num_of_words; i++)
    {
        if(d1->words[i] != d2->words[i])
            return 0;
    }
    return 1;
}


sforest sf_create_sforest()
{
    int i;
    sforest forest = (sforest) calloc(DICT_SIZE, sizeof(sfnode));
    for(i = 0; i < DICT_SIZE; i++)
    {
        forest[i] = sf_create_sfnode(i);
    }
    return forest;
}


/* dat tells the number of children which the node will have*/
fptree sf_create_fptree(data_type dat)
{

    fptree new_tree = calloc(1, sizeof(struct fptree_node));
    if(new_tree == NULL)
    {
        printf("new_tree calloc failed\n");
    }

    fpnode node = calloc(1, sizeof(struct fp_node));

    if(node == NULL)
    {
        printf("node calloc failed\n");
    }

    node->data_item = dat;
    node->ftid = INT_MAX;
    node->ltid = -1;

    new_tree->root = node;
    new_tree->head_table = NULL;
    return new_tree;
}


/* dat tells the number of children which the node will have*/
sfnode sf_create_sfnode(data_type dat)
{
    sfnode node = calloc(1, sizeof(struct sf_node));

    if(node == NULL)
    {
        printf("node calloc failed\n");
    }

    node->children = calloc(last_index(dat), sizeof(sfnode));
    node->data_item = dat;
    node->ftid = INT_MAX;
    node->ltid = -1;
    return node;
}


void sf_delete_sftree_structure(sfnode current_node)
{
    if(current_node == NULL)
        return;
    sfnode* current_child_ptr = current_node->children;
    sfnode this_child, temp;
    buffer buff = current_node->bufferhead;
    int idx;

    sf_delete_fptree(current_node->fptree);
    sf_delete_buffer(buff); /* clear up the buffer*/
    // free(current_node->bufferhead);
    // free(current_data_ptr);

    if(current_child_ptr != NULL)
    {
        for(idx = 0; idx < last_index(current_node->data_item); idx++)
        {
            if(current_child_ptr[idx] != NULL)
            {
                this_child = current_child_ptr[idx];
                sf_delete_sftree_structure(this_child);
                free(this_child); /* let the child go*/
                this_child = NULL;

                // data temp_data = current_data_ptr[idx];
                // free(temp_data); /* clear the data items*/
                // temp_data = NULL;
                current_child_ptr[idx] = NULL;
                // free(current_child_ptr);
                // current_data_ptr[idx] = NULL;
            }
        }
        free(current_node->children);
    }
    return;
}


void sf_delete_fptree_structure(fpnode current_node)
{
    if(current_node == NULL)
        return;

    fpnode this_child, temp;
    int idx;
    this_child = current_node->child;

    while(this_child)
    {
        temp = this_child->next;

        if(this_child->prev_similar != NULL)
            (this_child->prev_similar)->next_similar = this_child->next_similar;

        if(this_child->next_similar != NULL)
            (this_child->next_similar)->prev_similar= this_child->prev_similar;

        if(this_child->hnode && this_child->hnode->first == this_child)
            this_child->hnode->first = NULL;
        
        if(this_child->next)
            this_child->next = this_child->next->next;
        
        sf_delete_fptree_structure(this_child);
        free(this_child->child);
        this_child->child = NULL;
        free(this_child);
        // free(this_child->next);
        this_child = temp;
    }
    current_node->child = NULL;
    // free(current_node);
    return;
}


void sf_delete_data(data d)
{
    bardestroy(d);
    return;
}


void sf_delete_buffer(buffer head)
{
    if(head == NULL)
        return;
    buffer next;
    while(head != NULL)
    {
        next = head->next;
        sf_delete_data(head->itemset);
        free(head);
        head = NULL;
        head = next;
    }
    return;
}


void sf_delete_header_table(header_table* h)
{
    if(h == NULL)
        return;
    int idx, root_data = h[0]->data_item;
    header_table temp;
    for(idx = 0; idx < last_index(root_data); idx++)
    {
        temp = h[idx];
        if(temp)
            free(temp);
        temp = NULL;
    }
    free(h);
    h = NULL;
    return;
}


void sf_delete_fptree(fptree tree)
{
    if(tree == NULL)    return;
    sf_delete_header_table(tree->head_table);
    sf_delete_fptree_structure(tree->root);
    tree->root = NULL;
    free(tree);
    tree = NULL;
    return;
}


void sf_delete_sforest(sforest forest)
{
    int idx;
    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        sf_delete_sftree_structure(forest[idx]);
    }
    return;
}

// /****************************************************************************/

// creates a new node and inserts it into current_node
void sf_create_and_insert_new_child(sfnode current_node, data_type d, int tid)
{
    sfnode new_node = calloc(1, sizeof(struct sf_node));
    // we have pointer of children and children themselves
    new_node->children = calloc(last_index(d), sizeof(sfnode));
    new_node->data_item = d;
    new_node->parent = current_node;
    new_node->ftid = tid;
    new_node->ltid = tid;

    int idx = index(d, current_node->data_item);
    assert(current_node->children[idx] == NULL);
    current_node->children[idx] = new_node;
    return;
}


// explicitely inserts the child node in the current node
void sf_insert_new_child(fpnode current_node, fpnode new_child, int d)
{
    new_child->parent = current_node;
    fpnode temp = current_node->child;
    new_child->next = temp;
    current_node->child = new_child;
    return;
}


sfnode sf_copy_node(sfnode current_node, data_type d)
{
    sfnode new_node = calloc(1, sizeof(struct sf_node));
    // we have pointer of children and children themselves
    d < 0 ? new_node->children = NULL : (new_node->children = calloc(last_index(d), sizeof(sfnode)));
    // new_node->item_list = calloc(last_index(d->data_item), sizeof(data));
    new_node->data_item = current_node->data_item;
    new_node->parent = current_node->parent;
    new_node->ftid = current_node->ftid;
    new_node->ltid = current_node->ltid;
    return new_node;
}
// /****************************************************************************/

int sf_no_sfchildren(sfnode current_node)
{
    sfnode* current_child_ptr = current_node->children;
    int idx, no_children = 0;
    if(current_child_ptr)
    {
        for(idx = 0; idx < last_index(current_node->data_item); idx++)
        {
            if(current_child_ptr[idx])
                no_children++;
        }
    }

    return no_children;
}


int sf_no_fpchildren(fpnode current_node)
{
    fpnode child = current_node->child;
    int idx, no_children = 0;
    if(child != NULL)
    {
        while(child != NULL)
        {
            no_children++;
            child = child->next;
        }
    }

    return no_children;
}


int sf_get_height(sfnode node)
{
    int height = 0;
    while(node->parent != NULL)
    {
        height++;
        node = node->parent;
    }
    return height;
}


void sf_append_buffer(sfnode curr, data d, double freq, int tid)
{
    /* takes care of cases when to be buffered item is NULL*/
    /* this tid is the timestamp when the dataitem was buffered*/
    bit data_item;
    if(bit_array_find_next_set_bit(d, 0, &data_item) == 0)
        return;

    buffer last = curr->buffertail, temp_buff = curr->bufferhead;

    while(temp_buff)
    {
        temp_buff->freq *= pow(DECAY, tid - temp_buff->ltid);
        temp_buff->ltid = tid;
        if(sf_is_equal(temp_buff->itemset, d))
        {
            // printf("appending at node!\n");
            // sf_print_node(curr);
            temp_buff->freq += freq;;
            return;
        }
        temp_buff = temp_buff->next;
    }

    buffer new = (buffer) calloc(1, sizeof(struct buffer_node));

    if(last == NULL)
    {
        assert(curr->bufferhead == NULL); /* if tail is NULL then head must also be*/
        curr->bufferhead = new;
        curr->buffertail = new;
    }

    /* we create a new copy of the datanode*/
    new->ftid = tid;
    new->ltid = tid;
    new->itemset = bit_array_clone(d); /* copy the transaction*/
    new->freq = freq;

    if(last)
    {
        last->next = new;
        new->prev = last;
    }

    curr->buffertail = new;
    curr->buffertail->next = NULL;
    curr->bufferSize++;
    return;
}


buffer sf_pop_buffer(sfnode curr)
{
    buffer temp = curr->bufferhead, prev = NULL;
    if(temp == NULL)
    {
        assert(curr->buffertail == NULL);
        return NULL;
    }
    curr->bufferSize--;
    prev = curr->buffertail->prev;
    temp = curr->buffertail;

    if(prev)
        prev->next = NULL;
    else
        curr->bufferhead = NULL; /* we are popping off the only buffer present*/

    // assert(prev->next == curr->buffertail);

    curr->buffertail = prev;
    assert(temp->next == NULL);
    return temp;
}


void sf_fp_insert(fpnode current_node, header_table* htable, data d, double cnt, int tid)
{
    // put_in_buffer tells whether we want to ignore the buffer signal or not
    // d is a single item here and not an itemset
    // if the flag is up then we insert the remaining itemset into the bufferlist of that node and reset the flag
    assert(current_node != NULL);
    // assert(htable != NULL);
    int idx, root_data;
    data_type this_data = current_node->data_item;
    root_data = htable[0]->data_item;
    idx = index(this_data, root_data);
    header_table curr_header_node;

    // create the links right here only
    if(htable[idx] == NULL)
    {
        htable[idx] = calloc(1, sizeof(struct header_table_node));
        htable[idx]->data_item = this_data;
        htable[idx]->first = NULL;
        htable[idx]->cnt = 0.0;
        htable[idx]->ftid = INT_MAX - 1;
        htable[idx]->ltid = -1;
    }
    curr_header_node = htable[idx];
    assert(this_data == curr_header_node->data_item);

    if(htable && current_node->hnode == NULL && current_node->data_item != -1)
    {
        //append to the head of the linked list for this data item
        current_node->next_similar = curr_header_node->first;
        current_node->hnode = curr_header_node;
        if(curr_header_node->first)
            curr_header_node->first->prev_similar = current_node;

        curr_header_node->first = current_node;

        /* The first node next to the header table has NO previous node although a
          next node from the header table points to it*/
        current_node->prev_similar = NULL;
        // curr_header_node->cnt += current_node->freq;
    }
    curr_header_node->ftid = min(curr_header_node->ftid, tid);
    curr_header_node->cnt *= pow(DECAY, tid - curr_header_node->ltid);
    curr_header_node->cnt += cnt;
    curr_header_node->ltid = tid;
    if(current_node->data_item != root_data)
    	assert(current_node->parent != NULL);

    /* updating the frequency of the node according to the formula*/
    current_node->freq *= pow(DECAY, tid - current_node->ltid);
    current_node->freq += cnt;
    current_node->ltid = tid;
    current_node->ftid = min(tid, current_node->ftid);

    bit data_item;
    if(bit_array_find_next_set_bit(d, 0, &data_item) == 0)
        return;    //terminate when all items have been inserted

    // iterate through children
    // if the next data item has already occurred earlier, go along that child
    // otherwise make a new child
    fpnode current_child_ptr = current_node->child, prev = NULL, temp = NULL;
    idx = index(data_item, current_node->data_item);

    while(current_child_ptr && current_child_ptr->data_item != data_item)
    {
        prev = current_child_ptr;
        current_child_ptr = current_child_ptr->next;
    }

    if(current_child_ptr == NULL)
    {
        // copying the functionality of 'sf_create_and_insert_new_child(current_node, d, tid)'
        fpnode new_node = calloc(1, sizeof(struct sf_node));
        // printf("created node: %d\n", d->data_item);
        temp = current_node->child;
        new_node->next = temp;
        current_node->child = new_node;
        new_node->parent = current_node;
        new_node->data_item = data_item;
        new_node->ftid = tid;
        new_node->ltid = tid;
        current_child_ptr = new_node;
    }
    // assert(current_child_ptr[idx] != NULL);

    // printf("could not find appropriate child for %d:(\n", current_node->data_item);
    // data this_data_item = current_data_ptr;
    bit_array_clear_bit(d, data_item); /* equivalent to sending d->next*/
    sf_fp_insert(current_child_ptr, htable, d, cnt, tid);
    return;
}


void sf_insert_itemset_helper(sfnode node, int root_data, int tid)
{
    /* currently node is the root node*/
    assert(node != NULL);

    // sf_print_buffer(current_node);
    QStack* qstack = createQStack(); /* initialize a qstack*/
    push(qstack, node); /* push the root node in the qstack*/
    sfnode current_node, this_child;
    sfnode* current_child_ptr;
    data temp, d;

    int idx, no_trans = 0;
    bit temp_data, old_ptr, itemlen;

    while(qstack->size > 0) /* we go on till the time we have nodes in the stack*/
    {
        current_node = pop(qstack); /* get the node in LIFO manner ie. queue.
                                       This is to get the nodes level by level*/
        assert(current_node != NULL); /* since qstack is not empty, fetched node cant be null*/

        data_type this_data = current_node->data_item;
        idx = index(this_data, root_data); /* this index will point to the correct header node and the child node*/

        buffer popped = sf_pop_buffer(current_node); /* popped is the last buffer, it has the itemset
                                                        which will be propagated down*/
        if(popped)
            assert(popped->next == NULL); /* 'popped' is the buffer node*/
        else
            continue;

        if(current_node->bufferSize > 0) /* push the node back in the qstsack if it's buffer is still not empty*/
            push(qstack, current_node);

        /* updating the inner fields of the buffer*/
        popped->freq *= pow(DECAY, tid - popped->ltid);
        popped->ltid = tid;

        /* this controls pre-emption*/
        if(LEAVE_AS_BUFFER)
        {
            /* inserted in LIFO order*/
            // printf("leaving in buffer at node %d: ", current_node->data_item);
            // sf_print_data(d);
            current_node->bufferSize++;
            // sf_append_buffer(current_node, popped->itemset, tid); /* push back the popped buffer as we have to leave now*/
            LEAVE_AS_BUFFER = 0;
            delete_qstack(qstack);
            return;
        }

        /* this is to insert the transaction in the fptree*/
        if(current_node->fptree == NULL && (sf_get_height(current_node) >= LEAVE_LVL) /*some decision*/)
        {
            /* this is because we dont want the children array in fp-tree nodes*/
            current_node->fptree = sf_create_fptree(DICT_SIZE + 1);
            current_node->fptree->root->data_item = current_node->data_item;
            sf_create_header_table(current_node->fptree, tid);
            // current_node->fptree->root->data_item = -1;
            // sf_print_node(current_node);
            // printf("*************************************************\n");
        }

        if(current_node->fptree != NULL)
        {
            sf_fp_insert(current_node->fptree->root, current_node->fptree->head_table, popped->itemset,\
                         popped->freq, tid);
        }

        else
        {
            current_child_ptr = current_node->children; /* we will insert the itemsets in the
                                                           buffers of the children of current_node*/
            temp = bit_array_clone(popped->itemset);
            old_ptr = 0;
            itemlen = bit_array_num_bits_set(temp);

            /* this code readies the buffer for children of current_node*/
            while(itemlen)
            {
                bit_array_find_next_set_bit(temp, old_ptr, &temp_data);
                old_ptr = temp_data + 1;
                idx = index(temp_data, current_node->data_item); /* idx has the correct indices of
                                                                    the children of current_node*/

                /* this code just creates the nodes*/
                if(current_child_ptr[idx] == NULL)
                {
                    /*data item has to be inserted as new child*/
                    assert(current_child_ptr[idx] == NULL);
                    // assert(current_data_ptr[idx] == NULL);
                    sf_create_and_insert_new_child(current_node, temp_data, tid);
                }

                assert(current_node->children[idx] != NULL); /* we just updated that child!*/

                this_child = current_child_ptr[idx];
                // data this_data_item = current_data_ptr[idx];
                // assert(is_equal(this_data_item, temp));

                bit_array_clear_bit(temp, temp_data); /* move the buffer node forward*/

                if(itemlen > 1) /* here we are filling up the buffers of the children*/
                {
                    sf_append_buffer(current_child_ptr[idx], temp, popped->freq, tid);
                }

                /* updating the frequency of the node according to the formula*/
                current_child_ptr[idx]->freq *= pow(DECAY, tid - current_child_ptr[idx]->ltid);
                assert(popped->ltid == tid); /* updated just after it was retrieved*/
                current_child_ptr[idx]->freq += popped->freq;
                current_child_ptr[idx]->ltid = tid;
                itemlen--;
            }

            bit_array_free(temp);

            /* pushing the children in the qstack so that the above procedure can be applied on them*/
            temp = bit_array_clone(popped->itemset);
            old_ptr = 0;
            itemlen = bit_array_num_bits_set(temp);

            while(itemlen > 1) /* we wont go all the way upto the last node as it has already been updated*/
            {
                bit_array_find_next_set_bit(temp, old_ptr, &temp_data);
                old_ptr = temp_data + 1;

                idx = index(temp_data, current_node->data_item);
                sf_prune_buffer(current_child_ptr[idx], tid);

                bit_array_clear_bit(temp, temp_data); /* move the buffer node forward*/
                itemlen--;

                if(current_child_ptr[idx]->freq > EPS*(tid - current_child_ptr[idx]->ftid))
                {
                    // printf("not pruning freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
                    current_child_ptr[idx]->ltid = tid;
                    double toss = ((double) rand())/RAND_MAX;
                    if((toss < CARRY && current_child_ptr[idx]->fptree == NULL) || CARRY == 2.0)
                    /* this is to ensure that the itemset is inserted when we are emptying the buffer.
                    Propagate the node downwards with a certain probability
                    this reduces the length of qstack and makes insertion faster
                    but we'll need to empty the nodes later on
                    */
                    {
                        push(qstack, current_child_ptr[idx]);
                    }
                }
                else
                {
                    // printf("freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
                    sf_delete_sftree_structure(current_child_ptr[idx]);
                    current_child_ptr[idx] = NULL;
                }
            }
            bit_array_free(temp);
        }
        /* free the popped buffer to save space*/
        sf_delete_buffer(popped);
        // free(popped);
    }

    delete_qstack(qstack);
    return;
}


void sf_insert_itemset(sforest forest, data d, int tid)
{
    bit old_pos = 0, res = 0, size = bit_array_num_bits_set(d);

    while(old_pos < DICT_SIZE && bit_array_find_next_set_bit(d, old_pos, &res))
    {
        /* res has the index of data item*/
        old_pos = res + 1;
        bit_array_clear_bit(d, res); /* equivalent of passing the projection of the itemset*/
        sfnode root = forest[res];

        root->freq *= pow(DECAY, tid - root->ltid);
        root->ftid = min(root->ftid, tid);
        root->ltid = tid;
        root->freq++;

        if(size == 1) /* res is the last item*/
        {
            return;
        }
        sf_append_buffer(root, d, 1, tid); /* transactio: acdef, node a will have 'cdef'*/

        double toss = ((double) rand())/RAND_MAX;
        if(toss < CARRY || CARRY == 2.0)
            sf_insert_itemset_helper(root, root->data_item, tid);
        size--;
    }
    return;
}

/***************************************************************************************/
/* Various mining functions*/

/* do end = -1 and collected = NULL to print buff onto the file*/
int sf_print_patterns_to_file(int* collected, buffer buff, double cnt, int end, int pattern)
{
    FILE *sf;
    if(pattern == 0)
        sf = fopen("intermediate", "a");
    else if(pattern ==1)
        sf = fopen("output", "a");
    else if(pattern == 2)
        sf = fopen(OUT_FILE, "a");

    double minsup = pattern > 0 ? (pattern == 2 ? SUP : MINSUP_FREQ) : MINSUP_SEMIFREQ;
    minsup *= N;
    int pttrn_cnt = 0;

    if(sf == NULL)
        exit(0);

    /* this part is to print the transaction when the end node has no fptree*/
    {
        int t = 0;
        fprintf(sf, "%d", end + 1);

        while(t <= end)
        {
            fprintf(sf, " %d", collected[t]);
            t++;
        }
        pttrn_cnt++;

        if(pattern%2 == 0)
        {
            fprintf(sf, " %lf", cnt);
        }
        fprintf(sf, "\n");
    }

    data temp;
    bit data_item, old_pos;
    while(buff)
    {
        int t = 0, len = 0, new_end = end;
        old_pos = 0;

        while(buff && buff->ltid < minsup)
            buff = buff->next;

        if(buff == NULL)
            break;

        temp = buff->itemset;
        // sf_sort_data(buff->itemset, NULL);
        len = bit_array_num_bits_set(temp);

        /* this is to prevent the cases when the last transaction in collected matches the
           first transaction in temp*/
        bit_array_find_next_set_bit(temp, 0, &data_item);
        if(data_item == collected[end])
        {
            // temp = temp->next;
            new_end--;
        }
        fprintf(sf, "%d", new_end + len + 1);
        pttrn_cnt++;

        if(collected)
        {
            // fprintf(sf, " collected:");
            while(t <= new_end)
            {
                fprintf(sf, " %d", collected[t]);
                // printf(" %d", collected[t]);
                t++;
            }
        }

        // fprintf(sf, " -- fpmined trans:");

        while(old_pos < DICT_SIZE && bit_array_find_next_set_bit(temp, old_pos, &data_item))
        {
            fprintf(sf, " %d", (int) data_item);
            old_pos = data_item + 1;
            bit_array_clear_bit(temp, data_item);
        }

        // fprintf(sf, "  cnt = %lf\n", cnt < 0 ? buff->freq : min(cnt, buff->freq));

        if(pattern%2 == 0)
        {
            fprintf(sf, " %lf\n", cnt < 0 ? buff->freq : min(cnt, buff->freq));
            // printf(" %lf\n", node->freq);
        }
        buff = buff->next;
    }

    fclose(sf);
    return pttrn_cnt;
}


int sf_mine_frequent_itemsets_helper(sfnode node, int* collected, int end, int tid, int pattern)
{
    if(node == NULL)
        return;
    // sf_print_node(node);
    node->freq *= pow(DECAY, tid - node->ltid);
    int cnt = 0;
    double minsup = pattern>0 ? (pattern == 2 ? SUP : MINSUP_FREQ) : MINSUP_SEMIFREQ;
    if(node->freq >= N*minsup)
    {
        collected[++end] = node->data_item;
        if(end >= 0)
        {
            // printf("\nprinting fptree at node\n");
            // sf_print_node(node);
            /* there is an FPTree at this node*/
            sfnode collect_node = calloc(1, sizeof(struct sf_node));
            if(node->fptree != NULL)
            {
                data_type sorted = node->fptree->root->data_item + 1;
                // printf("\n****mining this FP-tree****\n");
                // sf_print_tree(node->fptree->root);
                // printf("****HEADER TABLE:****\n");
                // sf_print_header_table(node->fptree->head_table);
                /* change the root data to -1 for mining correctly*/
                node->fptree->root->data_item = -1;
                sf_fp_mine_frequent_itemsets(node->fptree, sorted, NULL, collect_node,\
                                             tid, N*minsup);
                /* reset the old (correct) value of root node data*/
                node->fptree->root->data_item = node->fptree->head_table[0]->data_item;
                // int i;
                // printf("collected: ");
                // for(i = 0; i < end + 1; i++)
                //  printf("%d, ", collected[i]);
                // printf("\n****node->freq = %lf, collected from tree****\n", node->freq);
                // sf_print_buffer(collect_node->bufferhead);
            }
            cnt += sf_print_patterns_to_file(collected, collect_node->bufferhead, node->freq, end, pattern);
            sf_delete_sftree_structure(collect_node);
            free(collect_node);
        }
        int idx;
        sfnode this_child;
        for(idx = 0; idx < last_index(node->data_item); idx++)
        {
            this_child = node->children[idx];
            if(this_child)
                cnt += sf_mine_frequent_itemsets_helper(this_child, collected, end, tid, pattern);
        }
    }
    return cnt;
}


int sf_mine_frequent_itemsets(sforest forest, int tid, int pattern)
{
    int idx, cnt = 0;
    int* collected = calloc(DICT_SIZE, sizeof(int));
    double minsup = pattern>0 ? (pattern == 2 ? SUP : MINSUP_FREQ) : MINSUP_SEMIFREQ;
    // printf("mining the tree with support: %lf\n", N*minsup);

    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        sfnode root = forest[idx];
        // printf("root = %d, freq = %lf\n", root->data_item, root->freq);
        cnt += sf_mine_frequent_itemsets_helper(root, collected, -1, tid, pattern);
    }
    free(collected);
    return cnt;
}


fpnode sf_dfs(fpnode node, header_table* htable, data_type highest_priority_data_item)
{
    if(node->touched == 0)    return NULL;

    header_table curr_head_table_node;
    int root_data = htable[0]->data_item;
    int idx = index(node->data_item == -1 ? root_data:node->data_item, root_data);

    // curr_head_table_node = htable[idx];

    if(htable[idx] == NULL)
    {
        htable[idx] = calloc(1, sizeof(struct header_table_node));
        htable[idx]->data_item = node->data_item;
        htable[idx]->first = NULL;
        htable[idx]->cnt = 0.0;
        htable[idx]->ftid = INT_MAX - 1;
        htable[idx]->ltid = -1;
    }


    curr_head_table_node = htable[idx];
    fpnode new_node = calloc(1, sizeof(struct sf_node));

    if(new_node->hnode == NULL)
    {
        //append to the head of the linked list for this data item
        new_node->next_similar = htable[idx]->first;
        new_node->hnode = htable[idx];
        if(htable[idx]->first)
            htable[idx]->first->prev_similar = new_node;

        htable[idx]->first = new_node;
        /*The first node next to the header table has NO previous node although a
        next node from the header table points to it*/
        new_node->prev_similar = NULL;
        // htable[idx]->cnt += node->freq;
    }
    // new_node->child = NULL;

    new_node->freq = node->touched;
    curr_head_table_node->cnt += new_node->freq;
    new_node->ltid = node->ltid; // will see this later
    new_node->data_item = node->data_item;

    if(new_node->data_item == highest_priority_data_item)
    {
        node->touched = 0;
        return new_node;
    }

    fpnode temp_child_list = node->child;
    int last = node->data_item >= 0 ? last_index(node->data_item) : last_index(root_data);

    while(temp_child_list)
    {
        // printf("adding child!\n");
        fpnode this_child = temp_child_list;
        if(this_child->touched > 0)
        {
            fpnode new_child = sf_dfs(this_child, htable, highest_priority_data_item);
            if(new_child == NULL)    continue;
            sf_insert_new_child(new_node, new_child, this_child->data_item);
        }
        temp_child_list = temp_child_list->next;
    }

    node->touched = 0;
    return new_node;
}


fptree sf_create_conditional_fp_tree(fptree tree, data_type data_item, double minsup, int tid)
{
    header_table curr_head_table_node;
    int root_data = tree->head_table[0]->data_item;
    int idx = index(data_item, root_data);

    curr_head_table_node = tree->head_table[idx];

    if(tree->head_table[idx] == NULL)
    {
        tree->head_table[idx] = calloc(1, sizeof(struct header_table_node));
        tree->head_table[idx]->data_item = data_item;
        tree->head_table[idx]->first = NULL;
        tree->head_table[idx]->cnt = 0.0;
        tree->head_table[idx]->ftid = tid;
        tree->head_table[idx]->ltid = tid;
    }

    curr_head_table_node = tree->head_table[idx];

    fpnode node = curr_head_table_node->first;
    // printf("prospects for %d\n", data_item);
    // printf("cnt = %lf, minsup = %lf\n", (curr_head_table_node->cnt)*\
            pow(DECAY, tid - curr_head_table_node->ltid), minsup);

    if(curr_head_table_node == NULL || (curr_head_table_node->cnt)*\
       pow(DECAY, tid - curr_head_table_node->ltid) < minsup)
    {
        // printf("rejecting %d\n", data_item);
        return NULL;
    }

    if(node == NULL)
    {
        // printf("first node is null!!\n");
        return NULL;
    }

    // printf("curr_head_table_node->cnt = %lf\n", curr_head_table_node->cnt);
    // printf("hpi = %d, %d, %lf\n", data_item, node->data_item, node->freq);

    /* node is the link to successive sfnodes having data type 'data_item'*/
    /* iterate through it and for each node in it, start from that node*/
    /* and touch all nodes till the root*/
    /* touched nodes are a means of identifying which nodes should be in coditional FP-tree*/
    double add;
    fpnode temp;
    while(node != NULL)
    {
        temp = node;
        add = (temp->freq)*pow(DECAY, tid - temp->ltid);
        while(temp != NULL)
        {
            temp->touched += add;
            // printf("touched = %lf\n", temp->touched);
            temp = temp->parent;
        }
        node = node->next_similar;
    }

    // printf("after touching:\n");
    // sf_print_tree(tree->root);

    fptree cond_tree = sf_create_fptree(DICT_SIZE + 1); /* this is because in FP-Trees we don't need the children array*/
    cond_tree->root->data_item = tree->head_table[0]->data_item;
    cond_tree->head_table = NULL;
    sf_create_header_table(cond_tree, tid);
    free(cond_tree->root); /* as we are assigning a custom root later*/

    /* now run a DFS from the root of the given FP_tree, for all touched nodes,*/
    /* create a copy for the conditional FP-tree*/
    fpnode cond_fptree = sf_dfs(tree->root, cond_tree->head_table, data_item);
    if(cond_fptree == NULL)
    {
        // printf("condtree = NULL!!!\n");
        return NULL;
    }

    // printf("cond_sftree, highers_prty_item = %d\n", data_item);
    // sf_print_tree(cond_sftree);
    // printf("\n");

    cond_tree->root = cond_fptree;
    // printf("child data = %d\n", cond_sftree->child?cond_sftree->child->data_item:0);
    // sf_print_header_table(cond_tree->head_table);
    return cond_tree;
}


void sf_fp_mine_frequent_itemsets(fptree tree, data_type sorted, data till_now, sfnode collected, int tid, double minsup)
{

    if(tree == NULL)    return;

    // if(till_now == NULL)
    //     printf("\nentered new mine with till_now = NULL and sorted = %d\n", sorted->data_item);
    // else if(sorted == NULL)
    //     printf("\nentered new mine sorted = NULL\n");
    // else
    //     printf("\nentered new mine sorted = %d\n", sorted->data_item);

    header_table curr_header_node;
    int idx;
    if(till_now == NULL)
        till_now = bit_array_create(DICT_SIZE);
    bit len_till_now = bit_array_num_bits_set(till_now);

    if(len_till_now)
    {
        // printf("till_now = ");
        // sf_print_data(till_now);

        //current itemset is not empty
        bit last_item;
        bit_array_find_last_set_bit(till_now, &last_item);

        /* get the proper index*/
        idx = index(last_item, tree->head_table[0]->data_item);

        curr_header_node = tree->head_table[idx];
        if(tree->head_table[idx] == NULL)
        {
            tree->head_table[idx] = calloc(1, sizeof(struct header_table_node));
            tree->head_table[idx]->data_item = last_item;
            tree->head_table[idx]->first = NULL;
            tree->head_table[idx]->cnt = 0.0;
            tree->head_table[idx]->ftid = INT_MAX;
            tree->head_table[idx]->ltid = -1;
        }

        curr_header_node = tree->head_table[idx];
        // sf_print_tree(tree->root);
        // printf("HEADER TABLE of cond_tree\n");
        // sf_print_header_table(tree->head_table);

        assert(curr_header_node != NULL);
        double new_cnt = (curr_header_node->cnt)*pow(DECAY, tid - curr_header_node->ltid);

        // printf("new_cnt=%lf\n", new_cnt);

        if(new_cnt >= minsup)
        {
            // append the frequent itemset to the buffer of a collector code
            // printf("mined: ");
            // sf_print_data(till_now);
            sf_append_buffer(collected, till_now, curr_header_node->cnt, tid);
            // printf("buffer of collected node in fp_mine: ");
            // sf_print_buffer(collected);
        }
    }

    if(sorted == DICT_SIZE)    return;

    //now check for supersets of this itemset by considering every next data item
    //in the sorted list
    data_type curr_data = sorted;
    while(curr_data != DICT_SIZE)
    {
        fptree cond_tree = sf_create_conditional_fp_tree(tree, curr_data,
                           minsup, tid);

        if(cond_tree == NULL)
        {
            // printf("skipped %d\n", curr_data->data_item);
            curr_data++;
            continue;
        }

        //append to front of current itemset
        bit_array_set_bit(till_now, curr_data);

        // printf("appended %d to till_now\n", curr_data->data_item);
        // printf("\nyolo %d cond_tree\n", curr_data->data_item);

        // if(cond_tree != NULL)
        //     sf_print_tree(cond_tree->root);

        // else
        //     printf("NULL tree\n");

        // if(curr_data->next != NULL)
        //     printf("going to mine %d\n", curr_data->next->data_item);
        // else
        //     printf("going to mine NULL\n");

        sf_fp_mine_frequent_itemsets(cond_tree, curr_data + 1, till_now, collected, tid, minsup);
        sf_delete_fptree(cond_tree);
        // if(curr_data->next != NULL)
        //     printf("finished mining %d\n", curr_data->next->data_item);
        // else
        //     printf("finished mining NULL\n");

        // printf("deleted %d from till_now\n", curr_data->data_item);

        //delete from front of current itemset
        bit_array_clear_bit(till_now, curr_data);
        curr_data++;
    }
    return;
}


/****************************************************************************/

/* this function fixes the next_similar variable ie. includes the new nodes in the next_similar scheme*/
void sf_create_header_table_helper(fpnode root, header_table* h)
{
    /* append this node to the corresponding list for this data item in the header table*/
    int idx;
    if(root->hnode == NULL && root->data_item != -1)
    {
        data_type this_data = root->data_item;
        idx = index(this_data, h[0]->data_item);
        header_table curr_header_node = h[idx];
        assert(curr_header_node->data_item == this_data);

        //append to the head of the linked list for this data item
        root->next_similar = curr_header_node->first;
        root->hnode = h[idx];

        if(curr_header_node->first)
            curr_header_node->first->prev_similar = root;

        curr_header_node->first = root;
        /* The first node next to the header table has NO previous node
           although a next node from the header table points to it*/
        root->prev_similar = NULL;
        /* frequency updates are taking place in update function*/
        // curr_header_node->cnt += root->freq;
    }

    fpnode current_child_ptr = root->child;
    while(current_child_ptr)
    {
        sf_create_header_table_helper(current_child_ptr, h);
        current_child_ptr = current_child_ptr->next;
    }
    return;
}


void sf_create_header_table(fptree tree, int tid)
{
    if(tree == NULL)    return;
    if(tree->head_table == NULL)
    {
        int cnt = 0;
        header_table* htable = calloc(last_index(tree->root->data_item), sizeof(header_table));
        header_table curr;

        /* just initialize the first entry in the header table*/
        htable[cnt] = calloc(1, sizeof(struct header_table_node));
        htable[cnt]->data_item = cnt + tree->root->data_item;
        htable[cnt]->first = NULL;
        htable[cnt]->cnt = 0.0;
        htable[cnt]->ftid = INT_MAX - 1;
        htable[cnt]->ltid = -1;
        tree->head_table = htable;
    }
    tree->root->parent = NULL;
    return;
}

/****************************************************************************/

void sf_empty_buffers(sforest forest, int tid)
{
    int root_data, idx, i, last;
    double oricarry = CARRY;
    sfnode current_node = NULL, root = NULL;
    QStack* qstack = createQStack();
    header_table* htable;
    CARRY = 2.0; /* now we will push all the nodes down otherwise\
                    we will lose itemsets in buffer only*/

    for(i = 0; i < DICT_SIZE; i++)
    {
        root = forest[i];
        root_data = root->data_item;
        push(qstack, root);
        if(root->bufferhead != NULL) /* push the buffered itemsets down*/
        {
            sf_insert_itemset_helper(root, root_data, tid);
        }

        else
        {
            while(qstack->size > 0)
            {
                current_node = pop(qstack);
                last = last_index(current_node->data_item);
                for(idx = 0; idx < last; idx++)
                {
                    if(current_node->children[idx] && current_node->children[idx]->bufferhead != NULL) /* if buffer is not empty then simply\
                                                                           call the insert function*/
                    {
                        root_data = current_node->children[idx]->data_item;
                        sf_insert_itemset_helper(current_node->children[idx], root_data, tid);
                    }
                    else
                        push(qstack, current_node->children[idx]);
                }
            }
        }
    }
    CARRY = oricarry;
    delete_qstack(qstack);
    return;
}


void sf_fp_merge(fpnode parent, fpnode child, int tid)
{

    // child's parent has been detached
    child->parent = parent;
    if(parent->child == NULL)
    {
        parent->child = child;
        return;
    }
    
    fpnode childptr = parent->child, prev = NULL;
    header_table htemp;
    data_type dat = child->data_item;

    while(childptr && childptr->data_item != dat)
    {
        prev = childptr;
        childptr = childptr->next;
    }

    if(childptr == NULL)
    {
        prev->next = child;
        prev->next->next = NULL;
        child->parent = parent;
        return;
    }

    /* this means that there is a node in the children of the parent which has the same data_item as child*/
    assert(childptr != NULL);

    childptr->freq = childptr->freq * pow(DECAY, tid - childptr->ltid)\
    					   + child->freq * pow(DECAY, tid - childptr->ltid);
    childptr->ltid = max(childptr->ltid, child->ltid);
    childptr->ftid = max(childptr->ftid, child->ftid);


    if(child->next_similar != NULL)
        (child->next_similar)->prev_similar = child->prev_similar;

    if(child->prev_similar != NULL)
        (child->prev_similar)->next_similar=child->next_similar;

    else
        child->hnode->first = child->next_similar;

    fpnode child_child = child->child;
    child->child = NULL;
    while(child_child)
    {
        // child_child->tree_node->parent = NULL;
        sf_fp_merge(childptr, child_child, tid);
        prev = child_child->next;
        // free(child_child);
        child_child = prev;
        prev = NULL;
    }
    free(child);
    child = NULL;
    return;
}


int sf_fp_prune(header_table* htable, int idx, int tid)
{
    header_table htemp = htable[idx];
    fpnode fir = htemp->first, temp2, parent;
    fpnode temp, ori, temp1;
    int root_data = htable[0]->data_item;

    while(fir != NULL)
    {
        parent = fir->parent;
	    if(fir->data_item == root_data)
	    {
	    	return 1;
	    }
        if(parent->child == NULL)
        	parent->child = fir;
        assert(parent->child != NULL);

        /* this code is separating the child from the parent and then we will merge\
           the parent and it's grandchildren*/
        if(parent->child == fir)
        {
            temp = parent->child;
            parent->child = parent->child->next;
        }
        else
        {
            temp = parent->child;
            ori = NULL;
            while(temp->next != NULL)
            {
                if(temp->next == fir)
                {
                    // ori is the child pointer which points to fir, so we must free it to save space
                    ori = temp->next;
                    temp->next = temp->next->next;
                    break;
                }
                temp = temp->next;
            }
        }

        /* temp holds a pointer to fir*/
        temp1 = fir->child;

        while(temp1 != NULL)
        {
            sf_fp_merge(parent, temp1, tid);
            // this is deleting the children pointers of fir as we will delete it eventually
            temp = temp1;
            temp1 = temp1->next;
            fir->child = temp1;
            temp->next = NULL;
            // free(temp); /* dont delete the child, it is being handled in the merge function*/
        }

        temp2 = fir;
        assert(fir != fir->next_similar);
        assert(fir->child == NULL);
        if(fir->next_similar)
            assert(fir->data_item == fir->next_similar->data_item);
        fir = fir->next_similar;
        // Do not do fp_delete_tree_structure(temp2); it will delete the children of temp2 also which we are merging!

        if(temp2->prev_similar!=NULL)
            (temp2->prev_similar)->next_similar = temp2->next_similar;

        if(temp2->next_similar!=NULL)
            (temp2->next_similar)->prev_similar = temp2->prev_similar;

        if(temp2->hnode && temp2->hnode->first == temp2)
            temp2->hnode->first = NULL;
    }
    htemp->first = NULL;
    return 0;
}


void sf_prune_buffer(sfnode curr, int tid)
{
    buffer curr_buff = curr->bufferhead, head = curr->bufferhead, temp;
    int root_data = curr->data_item, idx;
    if(head == NULL)
        return;

    bit data_item;

    /* delete the head if it needs to be deleted*/
    while(curr_buff == head && head != NULL)
    {
        bit_array_find_next_set_bit(curr_buff->itemset, 0, &data_item);
        idx = index(data_item, root_data);
        curr_buff->freq *= pow(DECAY, tid - curr_buff->ltid);
        if(curr->children[idx] == NULL && curr_buff->freq < EPS*(tid - curr_buff->ftid))
        {
            temp = curr_buff;
            head = head->next;
            if(head)
                head->prev = NULL;
            temp->next = NULL;
            curr->bufferSize--;
            sf_delete_buffer(temp);
        }
        curr_buff = curr_buff->next;
    }
    curr->bufferhead = head;
    if(head == NULL)
        curr->buffertail = NULL;

    /* B1<==>B2<==>[to_del]<==>B4<==>B5*/
    /* now we are beyond the point where heads need to be deleted*/
    while(curr_buff)
    {
        bit_array_find_next_set_bit(curr_buff->itemset, 0, &data_item);
        idx = index(data_item, root_data);
        curr_buff->freq *= pow(DECAY, tid - curr_buff->ltid);
        if(curr->children[idx] == NULL && curr_buff->freq < EPS*(tid - curr_buff->ftid))
        {
            if(curr_buff->next)
                curr_buff->next->prev = curr_buff->prev;
            curr_buff->prev->next = curr_buff->next;
            temp = curr_buff;
            if(curr->buffertail == temp)
                curr->buffertail = temp->prev;
            temp->next = NULL;
            curr->bufferSize--;
            sf_delete_buffer(temp);
        }
        curr_buff = curr_buff->next;
    }
    return;
}

/* Decay the count of node in the header table and decrease some value from nodes in the bltree*/
void sf_prune_helper(sfnode node, int root_data, int tid)
{
    node->freq *= pow(DECAY, tid - node->ltid);
    int child, idx, i;
    sfnode first, next, curr;
    QStack* qstack = createQStack();
    push(qstack, node);
    header_table* child_htable;
    while(qstack->size > 0)
    {
        node = get(qstack);
        sf_prune_buffer(node, tid);
        for(child = 0; child < last_index(node->data_item); child++)
        {
            if(node->children[child])
            {
                node->children[child]->freq *=\
                        pow(DECAY, tid - node->children[child]->ltid);
                if(node->children[child]->freq <= EPS*(tid - node->children[child]->ftid))
                {
                    sf_delete_sftree_structure(node->children[child]);
                    node->children[child] = NULL;
                }

                else
                {
                    push(qstack, node->children[child]);
	                if(node->children[child]->fptree)
	                {
	 					child_htable = node->children[child]->fptree->head_table;
	 					root_data = child_htable[0]->data_item;
	 					for(i = 0; i < last_index(root_data) && child_htable[i]; i++)
	 					{
	 						if(child_htable[i]->cnt < EPS*(tid - child_htable[i]->ftid))
	 						{
                    			// printf("child: %d, i: %d\n", child, i);
	 							if(sf_fp_prune(child_htable, i, tid))
	 							{
                    				sf_delete_fptree(node->children[child]->fptree);
                    				node->children[child]->fptree = NULL;
	                    			child_htable = NULL;
	                    			break;
	 							}
	                    	}
	 					}
	                }
	            }
            }
        }
    }
    delete_qstack(qstack);
    return;
}


void sf_prune(sforest forest, int tid)
{
    int i;
    for(i = 0; i < DICT_SIZE; i++)
    {
        sf_prune_helper(forest[i], forest[i]->data_item, tid);
    }
    return;
}

/****************************************************************************/



void sf_print_sfnode(sfnode node)
{
    if(node == NULL)    return;
    int c = sf_no_sfchildren(node), b = node->bufferSize;

    if(node->parent)
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = %d, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->ftid, node->ltid, node->parent->data_item, c, b);
    else
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = NULL, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->ftid, node->ltid, c, b);

    if(node->fptree)
    {
        printf("FPTREE:\n");
        sf_print_fptree(node->fptree->root);
        printf("******************************************************************************\n");
    }

    return;
}


void sf_print_fpnode(fpnode node)
{
    if(node == NULL)    return;
    int c = sf_no_fpchildren(node);

    if(node->parent)
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = %d, touched = %lf, children = %d\n", node->data_item, node->freq, node->ftid, node->ltid, node->parent->data_item, node->touched, c);
    else
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = NULL, touched = %lf, children = %d\n", node->data_item, node->freq, node->ftid, node->ltid, node->touched, c);

    return;
}


void sf_print_sforest(sforest forest)
{
    int i;
    for(i = 0; i < DICT_SIZE; i++)
    {
        printf("Printing SF-tree %d\n", forest[i]->data_item);
        printf("***************************************\n");
        sf_print_sfnode(forest[i]);
        printf("---------------------------------------\n\n");
    }
    return;
}


void sf_print_buffer(buffer head)
{
    buffer buff = head;
    while(buff)
    {
        printf("(tid: %d; freq: %lf) --> ", buff->ltid, buff->freq);
        sf_print_data(buff->itemset);
        buff = buff->next;
    }
    return;
}


void sf_print_fptree(fpnode node)
{
    sf_print_fpnode(node);
    fpnode temp = node->child;
    int idx;
    
    if(node->child)
    {
        while(temp)
        {
            sf_print_fptree(temp);
            temp = temp->next;
        }
    }
    return;
}


void sf_print_header_table(header_table* h)
{
    int idx;
    // z is the size of the table
    for(idx = 0; idx < last_index(h[0]->data_item) && h[idx] != NULL; idx++)
    {
        printf("item = %d, ftid = %d, ltid = %d, cnt = %lf\n", h[idx]->data_item, h[idx]->ftid, h[idx]->ltid, h[idx]->cnt);
    }
    return;
}


void sf_print_data(data d)
{
    bit result = 0, old_pos = 0;
    while(old_pos < DICT_SIZE && bit_array_find_next_set_bit(d, old_pos, &result))
    {
        printf("%d ", (int) result);
        old_pos = result + 1;
    }
    printf("\n");
    return;
}

/****************************************************************************/
