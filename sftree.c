/*
1. When deleting a child of a node, fix the child_list as well as the item_list
2. When removing nodes, also fix the header table
3. Header table is now not being recreated from scratch, it is being updated periodically using the sf_update_header_table function
4. But the tree is being constructed ie. next_similar and prev_similar pointers are being given periodically just before pruning
*/
#include "sftree.h"

int is_equal(data d1, data d2)
{
    return (d1->data_item == d2->data_item);
}


data sf_create_sorted_dummy()
{
    data d = NULL;
    int next = 0;
    while(next < DICT_SIZE)
    {
        data new_d = (data) calloc(1, sizeof(struct data_node));
        new_d->data_item = next++;
        new_d->next = d;
        d = new_d;
    }
    return d;
}

sforest sf_create_sforest()
{
    int i;
    sforest forest = (sforest) calloc(DICT_SIZE, sizeof(sftree));
    for(i = 0; i < DICT_SIZE; i++)
    {
        forest[i] = sf_create_sftree(i);
    }
    return forest;
}

sftree sf_create_sftree(data_type dat)
{

    sftree new_tree = calloc(1, sizeof(struct sftree_node));
    if(new_tree == NULL)
    {
        printf("new_tree calloc failed\n");
    }

    sfnode node = calloc(1, sizeof(struct sf_node));

    if(node == NULL)
    {
        printf("node calloc failed\n");
    }

    node->children = calloc(last_index(dat) , sizeof(sfnode));
    // node->item_list = calloc(last_index(dat), sizeof(data));
    node->data_item = dat;
    node->ftid = INT_MAX;
    node->ltid = -1;

    new_tree->root = node;
    new_tree->head_table = NULL;
    return new_tree;
}

void sf_delete_tree_structure(sfnode current_node)
{
    if(current_node == NULL)
        return;

    sfnode* current_child_ptr = current_node->children;
    data* current_data_ptr = current_node->item_list;
    sfnode this_child;
    buffer buff = current_node->bufferhead;
    int idx;

    for(idx = 0; idx < last_index(current_node->data_item); idx++)
    {
        if(current_child_ptr[idx] != NULL)
        {
            this_child = current_child_ptr[idx];

            if(this_child->prev_similar!=NULL)
                (this_child->prev_similar)->next_similar = this_child->next_similar;

            if(this_child->next_similar!=NULL)
                (this_child->next_similar)->prev_similar=(this_child)->prev_similar;

            if(this_child->hnode && this_child->hnode->first == this_child)
                this_child->hnode->first = NULL;

            sf_delete_tree_structure(this_child);
            free(this_child); /* let the children go*/
            this_child = NULL;

            // data temp_data = current_data_ptr[idx];
            // free(temp_data); /* clear the data items*/
            // temp_data = NULL;
            current_child_ptr[idx] = NULL;
            // current_data_ptr[idx] = NULL;
        }
    }
    free(current_child_ptr);
    free(current_data_ptr);
    sf_delete_buffer(buff); /* clear up the buffer*/
    current_node->bufferhead = NULL; /*free the buffer pointers*/
    current_node->buffertail = NULL;
}


void sf_delete_data_node(data d)
{
    data temp;
    while(d)
{
        temp = d;
        d = d->next;
        free(temp);
        temp = NULL;
    }
}

void sf_delete_buffer(buffer head)
{
    if(head == NULL)
        return;
    buffer next;
    while(head != NULL)
    {
        next = head->next;
        sf_delete_data_node(head->itemset);
        head = next;
    }
}

void sf_delete_header_table(header_table* h)
{
    int idx;
    header_table temp;
    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        temp = h[idx];
        if(temp)
            free(temp);
        temp = NULL;
    }
    free(h);
    h = NULL;
}


void sf_delete_sftree(sftree tree)
{
    if(tree == NULL)    return;
    sf_delete_tree_structure(tree->root);
    free(tree->root);
    tree->root = NULL;
    // sf_delete_header_table(tree->head_table);
    free(tree);
    tree = NULL;
}

void sf_delete_sforest(sforest forest)
{
    int idx;
    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        sf_delete_sftree(forest[idx]);
    }
}

int sf_size_of_sforest(sforest forest)
{
    int size = 0;
    int idx;
    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        size += sf_size_of_tree(forest[idx]->root);
    }
    return size;
}

int sf_size_of_tree(sfnode curr)
{
    if(curr == NULL)
        return 0;
    sfnode* child = curr->children;
    int size = sizeof(curr) + last_index(curr->data_item)*sizeof(curr->children), idx;
    buffer buff = curr->bufferhead;
    data temp;

    while(buff)
    {
        size += sizeof(buff);
        temp = buff->itemset;
        while(temp)
        {
            size += sizeof(temp);
            temp = temp->next;
        }
        buff = buff->next;
    }

    for(idx = 0; idx < last_index(curr->data_item); idx++)
    {
        if(child[idx])
            size += sf_size_of_tree(child[idx]);
    }
    return size;
}

long unsigned sf_no_of_nodes(sfnode curr, int freq)
{
    if(curr == NULL)
        return 0;
    long unsigned sum = 0;
    int i;
    for(i = 0; i < last_index(curr->data_item); i++)
    {
        if(curr->children[i] && curr->children[i]->freq == freq)
            sum++;
        sum += sf_no_of_nodes(curr->children[i], freq);
    }
    return sum;
}

/****************************************************************************/

// creates a new node and inserts it into current_node
void sf_create_and_insert_new_child(sfnode current_node, data d, int tid)
{
    sfnode new_node = calloc(1, sizeof(struct sf_node));
    // we have pointer of children and children themselves
    new_node->children = calloc(last_index(d->data_item), sizeof(sfnode));
    // new_node->item_list = calloc(last_index(d->data_item), sizeof(data));
    new_node->data_item = d->data_item;
    new_node->parent = current_node;
    new_node->ftid = tid;
    new_node->ltid = tid;

    int idx = index(d->data_item, current_node->data_item);

    // data new_data = calloc(1, sizeof(struct data_node));
    // new_data->data_item = d->data_item;
    // new_data->next = NULL;

    // //assert(sf_verify_node(new_node));
    assert(current_node->children[idx] == NULL);
    // assert(current_node->item_list[idx] == NULL);

    current_node->children[idx] = new_node;
    // current_node->item_list[idx] = new_data;
    //assert(sf_verify_node(current_node));
}

// explicitely inserts the child node in the current node
void sf_insert_new_child(sfnode current_node, sfnode new_child, data d)
{
    new_child->parent = current_node;
    data new_data = calloc(1, sizeof(struct data_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;
    int idx = index(d->data_item, current_node->data_item);

    current_node->children[idx] = new_child;
    // current_node->item_list[idx] = new_data;
    //assert(sf_verify_node(new_child));
    //assert(sf_verify_node(current_node));
}


/****************************************************************************///

int sf_no_children(sfnode current_node)
{
    sfnode* current_child_ptr = current_node->children;
    int idx, no_children = 0;
    for(idx = 0; idx < last_index(current_node->data_item); idx++)
    {
        if(current_child_ptr[idx])
            no_children++;
    }
    return no_children;
}


int sf_no_dataitem(sfnode current_node)
{
    // data* current_data_ptr = current_node->item_list;
    data * current_data_ptr = NULL;
    int idx, no_dataitem = 0;
    for(idx = 0; idx < last_index(current_node->data_item); idx++)
    {
        if(current_data_ptr[idx])
            no_dataitem++;
    }
    return no_dataitem;
}


int sf_verify_node(sfnode current_node)
{
    printf("\nverification for node %d:- ", current_node->data_item);

    sfnode* current_child_ptr;
    int flag = 1, res = 1, idx, i;
    // while(current_node->parent && current_child_ptr)
    // {
    //     if(current_child_ptr->tree_node == current_node)
    //         flag = 2;
    //     current_child_ptr = current_child_ptr->next;
    // }

    // if(flag == 2){
    //     printf("child is not a child of it's parent!");
    //     res = 0;
    // }
    if(sf_no_children(current_node) != sf_no_dataitem(current_node))
    {
        res = 0;
        printf("not equal number of children and itemlist: %d, %d\n", sf_no_children(current_node), sf_no_dataitem(current_node));
        return res;
    }

    current_child_ptr = current_node->children;
    data* current_data_ptr = current_node->item_list;

    for(i = 0; i < last_index(current_node->data_item); idx++)
    {
        if(current_data_ptr[i])
        {
            int x =  current_child_ptr[i]->data_item;
            int y =  current_data_ptr[i]->data_item;
            printf("c(%d, %d) ", i, x);
            printf("i(%d, %d) ", i, y);

            if(x != y)
                res = 0;
        }
    }
    if(res == 0)
        printf("inconsistant children and itemlist: %d, %d\n", sf_no_children(current_node), sf_no_dataitem(current_node));
    return res;
}

void sf_append_buffer(sfnode curr, data d, int tid)
{
    /* takes care of cases when to be buffered item is NULL*/
    /* this tid is the timestamp when the dataitem was buffered*/
    if(d == NULL)
        return;

    buffer last = curr->buffertail;
    buffer new = (buffer) calloc(1, sizeof(struct buffer_node));

    if(last == NULL)
    {
        assert(curr->bufferhead == NULL); /* if tail is NULL then head must also be*/
        curr->bufferhead = new;
        curr->buffertail = new;
    }
    /* we create a new copy of the datanode*/
    data temp;
    new->tid = tid;
    new->itemset = (data) calloc(1, sizeof(struct data_node));
    new->itemset->data_item = d->data_item;
    temp = new->itemset;
    d = d->next;
    while(d)
    {
        temp->next = (data) calloc(1, sizeof(struct data_node));
        temp = temp->next;
        temp->data_item = d->data_item;
        d = d->next;
    }
    temp->next = NULL;
    if(last)
        last->next = new;
    curr->buffertail = new;
    curr->buffertail->next = NULL;
}

buffer sf_pop_buffer(sfnode curr)
{
    buffer temp = curr->bufferhead, prev = NULL;

    if(temp == NULL)
    {
        assert(curr->buffertail == NULL);
        return NULL;
    }

    while(temp->next)
    {
        prev = temp;
        temp = temp->next;
    }

    // assert(prev->next == curr->buffertail);
    if(prev)
        prev->next = NULL;
    else
        curr->bufferhead = NULL; /* we are popping off the only buffer present*/

    curr->buffertail = prev;
    assert(temp->next == NULL);
    return temp;
}


sfnode sf_fp_insert(sfnode current_node, header_table htable, data d, int tid, int put_in_buffer)
{
    // put_in_buffer tells whether we want to ignore the buffer signal or not
    // d is a single item here and not an itemset
    // if the flag is up then we insert the remaining itemset into the bufferlist of that node and reset the flag
    assert(current_node != NULL);
    // assert(htable != NULL);

    // create the links right here only
    if(current_node->hnode == NULL && current_node->data_item != -1)
    {
        data_type this_data = current_node->data_item;
        header_table curr_header_node = htable;

        while(curr_header_node && curr_header_node->data_item != this_data)
            curr_header_node = curr_header_node->next;

        //append to the head of the linked list for this data item
        current_node->next_similar = curr_header_node->first;
        current_node->hnode = curr_header_node;
        if(curr_header_node->first)
            curr_header_node->first->prev_similar = current_node;

        curr_header_node->first = current_node;
        /*The first node next to the header table has NO previous node although a next node from the header table points to it*/
        current_node->prev_similar = NULL;
        // curr_header_node->cnt += current_node->freq;
    }


    //updating the frequency of the node according to the formula
    current_node->freq *= pow(DECAY, tid - current_node->tid);
    current_node->freq++;
    current_node->tid = tid;

    if(d == NULL)    return current_node;    //terminate when all items have been inserted

    // if(put_in_buffer == 1 && leave_as_buffer)
    // {
    //     printf("leaving in buffer at node %d: ", current_node->data_item);
    //     fp_print_data_node(d);
    //     buffer buff = current_node->itembuffer;
    //     buffer new = (buffer) calloc(1, sizeof(struct buffer_node));

    //     data temp;
    //     new->tid = tid;
    //     new->itemset = (data) calloc(1, sizeof(struct data_node));
    //     new->itemset->data_item = d->data_item;
    //     temp = new->itemset;
    //     d = d->next;
    //     while(d)
    //     {
    //         temp->next = (data) calloc(1, sizeof(struct data_node));
    //         temp = temp->next;
    //         temp->data_item = d->data_item;
    //         d = d->next;
    //     }
    //     temp->next = NULL;
    //     new->next = buff;
    //     current_node->itembuffer = new;
    //     current_node->bufferSize++;
    //     leave_as_buffer = 0;
    //     return current_node;
    // }


    //iterate through children
    //if the next data item has already occurred earlier, go along that child
    //otherwise make a new child
    sfnode* current_child_ptr = current_node->children, prev = NULL;
    data current_data_ptr = current_node->item_list;
    int idx = index(d->data_item, current_node->data_item);
    sfnode this_child = current_child_ptr[idx];

    if(this_child == NULL)
    {
        fp_create_and_insert_new_child(current_node, d, tid);
    }
    assert(current_child_ptr[idx] == NULL);
    
    // printf("could not find appropriate child for %d:(\n", current_node->data_item);
    data this_data_item = current_data_ptr;
    sfnode after_insert = sf_fp_insert(this_child, htable, d->next, tid, put_in_buffer);
    return current_node;
}


int sf_insert_itemset_helper(sfnode node, header_table* htable, int tid)
{
    // put_in_buffer tells whether we want to ignore the buffer signal or not
    // d is a single item here and not an itemset
    // if the flag is up then we insert the remaining itemset into the bufferlist of that node and reset the flag
    assert(node != NULL);
    extern int leave_as_buffer;

    // sf_print_buffer(current_node);
    QStack* qstack = createQStack();
    push(qstack, node);
    sfnode current_node;

    node->ftid = min(node->ftid, tid);

    node->freq *= pow(DECAY, tid - node->ltid);
    node->freq++;
    node->ltid = max(node->ltid, tid);
    
    int max = 0;

    while(qstack->size > 0)
    {
        if(qstack->size > max)
            max = qstack->size;

        current_node = get(qstack);
        assert(current_node != NULL); /* since qstack is not empty, fetched node cant be null*/

        buffer popped = sf_pop_buffer(current_node);
        if(popped)
            assert(popped->next == NULL); /* 'popped' is the last item*/

        data d = popped->itemset;

        /* this controls pre-emption*/
        if(leave_as_buffer)
        {
            /* inserted in LIFO order*/
            printf("leaving in buffer at node %d: ", current_node->data_item);
            sf_print_data_node(d);
            current_node->bufferSize++;
            leave_as_buffer = 0;
            delete_qstack(qstack);
            return;
        }

        sfnode* current_child_ptr = current_node->children;
        // data* current_data_ptr = current_node->item_list;
        data temp = popped->itemset;

        int idx;
        while(temp)
        {
            idx = index(temp->data_item, current_node->data_item);

            /* this code just creates the nodes*/
            if(current_child_ptr[idx] == NULL)
            {
                /*data item has to be inserted as new child*/
                assert(current_child_ptr[idx] == NULL);
                // assert(current_data_ptr[idx] == NULL);
                sf_create_and_insert_new_child(current_node, temp, tid);
            }

            assert(current_node->children[idx] != NULL);
            current_child_ptr[idx]->ltid = tid;

            sfnode this_child = current_child_ptr[idx];
            // data this_data_item = current_data_ptr[idx];
            // assert(is_equal(this_data_item, temp));

            if(temp->next)
                sf_append_buffer(current_child_ptr[idx], temp->next, tid);

            /*updating the frequency of the node according to the formula*/
            current_child_ptr[idx]->freq *= pow(DECAY, tid - current_child_ptr[idx]->ltid);
            current_child_ptr[idx]->freq++;
            
            /* update the tid of only the node which won't get updated later*/
            // if(temp->next == NULL)
            // {
            //     current_child_ptr[idx]->ltid = tid;
            // }
            
            temp = temp->next;
        }

        /* now apply the same procedure on all the children so further propagate the items*/
        temp = popped->itemset;

        while(temp->next) /* we wont go all the way upto the last node as it has already been updated*/
        {
            idx = index(temp->data_item, current_node->data_item);
            temp = temp->next;
            // if(current_child_ptr[idx]->freq > EPS*(tid - current_child_ptr[idx]->ftid))
            {
                // printf("not pruning freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
                current_child_ptr[idx]->ltid = tid;
                push(qstack, current_child_ptr[idx]);
            }
            // else
            // {
            //     // printf("freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
            //     sf_delete_tree_structure(current_child_ptr[idx]);
            //     current_child_ptr[idx] = NULL;
            // }
        }

        /* free the popped buffer to save space*/
        sf_delete_buffer(popped);
        free(popped);
    }

    delete_qstack(qstack);
    return max;
}


int sf_insert_itemset(sforest forest, data d, int tid)
{
    int max = 0;
    while(d)
    {
        sftree tree = forest[d->data_item];
        if(d->next == NULL)
        {
            tree->root->ftid = min(tree->root->ftid, tid);
            tree->root->ltid = max(tree->root->ltid, tid);
            tree->root->freq *= pow(DECAY, tid - tree->root->ltid);
            tree->root->freq++;
            // printf("%d\n", max);
            return max;
        }
        sf_append_buffer(tree->root, d->next, tid); /* transaction: acdef, node a will have 'cdef'*/
        int val = sf_insert_itemset_helper(tree->root, tree->head_table, tid);
        max = max < val ? val:max;
        d = d->next;
    }
}


/***************************************************************************************/

void sf_mine_frequent_itemsets_helper(sfnode node, int* collected, int end, int pattern)
{
    if(node == NULL)
        return;
    // sf_print_node(node);

    if((pattern == 0 && node->freq >= MINSUP_SEMIFREQ) ||
       (pattern == 1 && node->freq >= MINSUP_FREQ))
    {
        collected[++end] = node->data_item;
        if(end >= 0)
        {
            // sf_print_node(node);
            // printf("end = %d\n", end);
            // printf("**** end = %d ****\n", end);

            FILE *fp;
            if(pattern == 0)
                fp = fopen("intermediate", "a");
            else
                fp = fopen("output", "a");

            int t = 0;
            fprintf(fp, "%d", end + 1);
            // printf("%d", end + 1);

            while(t <= end)
            {
                fprintf(fp, " %d", collected[t]);
                // printf(" %d", collected[t]);
                t++;
            }

            if(pattern == 0)
            {
                fprintf(fp, " %lf", node->freq);
                // printf(" %lf\n", node->freq);
            }
            fprintf(fp, "\n");
            fclose(fp);
        }

        int idx;
        sfnode this_child;
        for(idx = 0; idx < last_index(node->data_item); idx++)
        {
            this_child = node->children[idx];
            if(this_child)
                sf_mine_frequent_itemsets_helper(this_child, collected, end, pattern);
        }
    }
}

void sf_mine_frequent_itemsets(sforest forest, int pattern)
{
    int idx;
    int* collected = calloc(DICT_SIZE, sizeof(int));

    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        sfnode root = forest[idx]->root;
        // printf("root = %d, freq = %lf\n", root->data_item, root->freq);
        sf_mine_frequent_itemsets_helper(root, collected, -1, pattern);
    }
    free(collected);
}

/****************************************************************************/


// this function fixes the next_similar variable also
void sf_create_header_table_helper(sfnode root, header_table* h)
{
    //append this node to the corresponding list for this data item in the header table
    if(root->hnode == NULL && root->data_item != -1)
    {
        data_type this_data = root->data_item;
        header_table curr_header_node = h[this_data];
        assert(curr_header_node->data_item == this_data);
        //append to the head of the linked list for this data item
        root->next_similar = curr_header_node->first;
        root->hnode = h[this_data];
        if(curr_header_node->first)
            curr_header_node->first->prev_similar = root;

        curr_header_node->first = root;
        /*The first node next to the header table has NO previous node although a next node from the header table points to it*/
        root->prev_similar = NULL;
        // curr_header_node->cnt += root->freq;
    }

    sfnode* current_child_ptr = root->children;
    int idx;
    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        if(current_child_ptr[idx])
        {
            sfnode this_child = current_child_ptr[idx];
            sf_create_header_table_helper(this_child, h);
        }
    }
}


/*  this function is updating the header table of the sftree*/
void sf_update_header_table(header_table* htable, data dat, int tid)
{
    header_table temp, prev;
    data tdat = dat;
    int idx;
    while(tdat)
    {
        idx = tdat->data_item;
        temp = htable[idx];
        assert(temp->data_item == tdat->data_item);
        temp->ftid = min(temp->ftid, tid);
        temp->ftid = max(temp->ltid, tid);
        temp->cnt = 0;
        sfnode nxtnode = temp->first;
        while(nxtnode)
        {
            nxtnode->freq *= pow(DECAY, tid-nxtnode->ltid);
            nxtnode->ltid = tid;
            temp->cnt += nxtnode->freq;
            nxtnode = nxtnode->next_similar;
        }
        tdat = tdat->next;
    }
}


void sf_create_header_table(sftree tree, int tid)
{
    if(tree == NULL)    return;
    if(tree->head_table == NULL)
    {
        int cnt;
        header_table* htable = calloc(DICT_SIZE, sizeof(header_table));
        header_table curr;

        for(cnt = 0; cnt < DICT_SIZE; cnt++)
        {
            htable[cnt] = calloc(1, sizeof(struct header_table_node));
            htable[cnt]->data_item = cnt;
            htable[cnt]->first = NULL;
            htable[cnt]->cnt = 0.0;
            htable[cnt]->ftid = INT_MAX;
            htable[cnt]->ltid = INT_MIN;
        }
        tree->head_table = htable;
    }
    tree->root->parent = NULL;
    sf_create_header_table_helper(tree->root, tree->head_table);

    data sorted = sf_create_sorted_dummy();
    sf_update_header_table(tree->head_table, sorted, tid);
}
/****************************************************************************/

// sorts the I-list in DESCENDING order
void sf_sort_header_table(header_table* htable, double* table)
{
    if(htable == NULL)
        return;

    header_table temp, nxt;
    sfnode tnode;
    double tcnt;
    data_type tdata_item;
    int idx1, idx2;

    // insertion sorting the header table
    for(idx1 = 0; idx1 < DICT_SIZE; idx1++)
    {
        for(idx2 = idx1+1; idx2 < DICT_SIZE; idx2++)
        {
            temp = htable[idx1];
            nxt = htable[idx2];
            if(temp->cnt < nxt->cnt)
            {
                tdata_item = temp->data_item;
                temp->data_item = nxt->data_item;
                nxt->data_item = tdata_item;

                tdata_item = temp->ltid;
                temp->ltid = nxt->ltid;
                nxt->ltid = tdata_item;

                tdata_item = temp->ftid;
                temp->ftid = nxt->ftid;
                nxt->ftid = tdata_item;

                tnode = temp->first;
                temp->first = nxt->first;
                nxt->first = tnode;

                tcnt = temp->cnt;
                temp->cnt = nxt->cnt;
                nxt->cnt = tcnt;

            }
        }
    }

    for(idx1 = 0; idx1 < DICT_SIZE; idx1++)
    {
        // +1 because the last_index -1 also exists
        temp = htable[idx1];
        table[temp->data_item + 1] = temp->cnt;
    }
}


data sf_reverse_data(data head)
{
    data prev = NULL, curr = head, temp;
    while(curr)
    {
        temp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = temp;
    }
    return prev;
}


// removes duplicate items also
void sf_sort_data(data head, double* arr)
{
    data temp = head, nxt, temp1, prev;
    data_type tdt, ori;
    int flag;
    for(; temp->next != NULL; temp = temp->next)
    {
        for(nxt = temp->next; nxt != NULL; nxt = nxt->next)
        {
            flag = 0;

            if(arr == NULL && temp->data_item > nxt->data_item)
                flag = 1;

            else if(arr && arr[temp->data_item + 1] < arr[nxt->data_item + 1])
                flag = 1;

            if(flag)
            {
                tdt = temp->data_item;
                temp->data_item = nxt->data_item;
                nxt->data_item = tdt;
            }
        }
    }
    /*
       Removing duplicate items here
    */
    ori = head->data_item;

    for(prev = head, temp = head->next; temp != NULL; temp = temp->next)
    {
        while(temp != NULL && temp->data_item == ori)
        {
            temp1 = temp->next;
            free(prev->next);
            prev->next = temp1;
            temp = temp1;
        }
        if(temp)
        {
            ori = temp->data_item;
            prev = temp;
        }
        else
            break;
    }
}


data sf_array_to_datalist(int* arr, int end)
{
    int i;
    data head = (data) calloc(1, sizeof(struct data_node));
    data temp = head;
    head->data_item = arr[1];
    head->next = NULL;
    for(i = 2; i <= end; i++)
    {
        temp->next = (data) calloc(1, sizeof(struct data_node));
        temp = temp->next;
        temp->data_item = arr[i];
        temp->next = NULL;
    }
    return head;
}


void sf_fix_touched(sfnode node)
{
    if(node == NULL)
        return;

    node->touched = 0;
    sfnode* child = node->children;
    int idx;

    for(idx = 0; idx < last_index(node->data_item); idx++)
    {
        if(child[idx])
            sf_fix_touched(child[idx]);
    }
}

void sf_update_ancestor(sfnode temp)
{
    sfnode temp1 = temp->parent;
    while(temp1->parent != NULL)
    {
        temp1->freq -= (temp->freq * pow(DECAY, temp1->ltid - temp->ltid));
        temp1=temp1->parent;
    }
}


void sf_empty_buffers(sfnode curr, header_table htable, int tid)
{
    return;
}


void sf_prune_helper(sfnode node, int tid)
{
    node->freq *= pow(DECAY, tid - node->ltid);
    int child;
    for(child = 0; child < last_index(node->data_item); child++)
    {
        if(node->children[child] && (node->children[child]->freq > EPS*(tid - node->children[child]->ftid)))
            sf_prune_helper(node->children[child], tid);
        else
        {
            sf_delete_tree_structure(node->children[child]);
            node->children[child] = NULL;
        }
    }
}

void sf_prune(sforest forest, int tid)
{
    int i;
    for(i = 0; i < DICT_SIZE; i++)
    {
        sf_prune_helper(forest[i]->root, tid);
    }
}

/****************************************************************************/


void sf_print_node(sfnode node)
{
    if(node == NULL)    return;
    int c = sf_no_children(node), b = node->bufferSize;

    if(node->parent)
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = %d, touched = %lf, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->ftid, node->ltid, node->parent->data_item, node->touched, c, b);
    else
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = NULL, touched = %lf, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->ftid, node->ltid, node->touched, c, b);

    printf("BUFFER:\n");
    buffer buff = node->bufferhead;
    while(buff)
    {
        // sf_print_data_node(buff->itemset);
        buff = buff->next;
    }
}

void sf_print_sforest(sforest forest)
{
    int i;
    for(i = 0; i < DICT_SIZE; i++)
    {
        printf("Printing SF-tree %d\n", forest[i]->root->data_item);
        sf_print_tree(forest[i]->root);
        printf("---------------------------------------\n\n");
    }
}


void sf_print_buffer(sfnode node)
{
    printf("NODE: %d, BUFFER:\n", node->data_item);
    buffer buff = node->bufferhead;
    while(buff)
    {
        sf_print_data_node(buff->itemset);
        buff = buff->next;
    }
}

void sf_print_tree(sfnode node)
{
    sf_print_node(node);
    sfnode* curr_child_list = node->children;
    int idx;
    for(idx = 0; idx < last_index(node->data_item); idx++)
    {
        if(curr_child_list[idx] != NULL)
            sf_print_tree(curr_child_list[idx]);
        // printf("going to child %d %d\n", this_child->data_item, curr_data->data_item);
    }
}


void sf_print_header_table(header_table* h)
{
    int idx;
    // z is the size of the table
    for(idx = 0; idx < last_index(h[idx]->data_item); idx++)
    {
        printf("item = %d, ftid = %d, ltid = %d, cnt = %lf\n", h[idx]->data_item, h[idx]->ftid, h[idx]->ltid, h[idx]->cnt);
        sfnode node = h[idx]->first;
        while(node != NULL && node->next_similar != node)
        {
            // sf_print_node(node);
            node = node->next_similar;
            // z++;
        }
        // printf("\n");
    }
}

void sf_print_data_node(data d)
{
    while(d)
    {
        printf("%d ", d->data_item);
        d = d->next;
    }
    printf("\n");
}

/****************************************************************************/
