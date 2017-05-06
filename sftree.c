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
    while(d1 != NULL && d2 != NULL)
    {
        if(d1->data_item != d2->data_item)
            return 0;
        d1 = d1->next;
        d2 = d2->next;
    }
    if(d1 == NULL && d2 == NULL)
        return 1;
    return 0;
}


/* sorted_dummy is also smartly indexed now*/
data sf_create_sorted_dummy(int start)
{
    data d = NULL;
    int next = start;
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
        sf_create_header_table(forest[i], INT_MAX);
    }
    return forest;
}


/* dat tells the number of children which the node will have*/
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

    node->children = calloc(last_index(dat), sizeof(sfnode));
    // node->item_list = calloc(last_index(dat), sizeof(data));
    node->data_item = dat;
    node->ftid = INT_MAX;
    node->ltid = -1;

    new_tree->root = node;
    new_tree->head_table = NULL;
    return new_tree;
}


void sf_create_update_header_node(header_table* htable, data_type d, int root_data, int tid)
{
    int idx = index(d, root_data); /* idx in the header table*/
    if(htable[idx] == NULL)
    {
        htable[idx] = calloc(1, sizeof(struct header_table_node));
        htable[idx]->data_item = d;
        htable[idx]->first = NULL;
        htable[idx]->cnt = 0.0;
        htable[idx]->ftid = INT_MAX - 1;
        htable[idx]->ltid = -1;
    }
    header_table curr_header_node = htable[idx];
    assert(d == curr_header_node->data_item);

    if(curr_header_node->ltid < tid)
    {
        curr_header_node->cnt++;
        curr_header_node->ftid = min(curr_header_node->ftid, tid);
        curr_header_node->ltid = tid;
    }
}


void sf_delete_tree_structure(sfnode current_node)
{
    if(current_node == NULL)
        return;
    sfnode* current_child_ptr = current_node->children;
    assert(current_node != current_node->child);
    // data* current_data_ptr = current_node->item_list;
    sfnode this_child, temp;
    buffer buff = current_node->bufferhead;
    int idx;

    sf_delete_sftree(current_node->fptree);
    sf_delete_buffer(buff); /* clear up the buffer*/
    // free(current_node->bufferhead);
    // free(current_data_ptr);

    if(current_node->child == NULL && current_child_ptr != NULL)
    {
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
                sf_delete_tree_structure(this_child->child);
                sf_delete_tree_structure(this_child->next);
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

    else
    {
        this_child = current_node->child;
        while(this_child)
        {
            temp = this_child->next;

            if(this_child->prev_similar != NULL)
                (this_child->prev_similar)->next_similar = this_child->next_similar;

            if(this_child->next_similar != NULL)
                (this_child->next_similar)->prev_similar=(this_child)->prev_similar;

            if(this_child->hnode && this_child->hnode->first == this_child)
                this_child->hnode->first = NULL;

            free(this_child->children);
            sf_delete_tree_structure(this_child);
            // free(this_child->child);
            free(this_child);
            // free(this_child->next);
            this_child = temp;
        }
        free(current_node->children);
    }
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
        free(head);
        head = NULL;
        head = next;
    }
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
}


void sf_delete_sftree(sftree tree)
{
    if(tree == NULL)    return;
    sf_delete_header_table(tree->head_table);
    sf_delete_tree_structure(tree->root);
    free(tree->root);
    tree->root = NULL;
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


double sf_size_of_sforest(sforest forest)
{
    double size = 0;
    int idx;
    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        size += sf_size_of_tree(forest[idx]->root);
    }
    return size;
}


double sf_size_of_tree(sfnode curr)
{
    if(curr == NULL)
        return 0;
    sfnode* child = curr->children;
    sfnode new_child = curr->child;

    double size = 1*sizeof(struct sf_node) + last_index(curr->data_item)*sizeof(curr->children);
    int idx;
    buffer buff = curr->bufferhead;
    data temp;

    while(buff)
    {
        size += sizeof(struct buffer_node);
        temp = buff->itemset;
        while(temp)
        {
            size += sizeof(struct data_node);
            temp = temp->next;
        }
        buff = buff->next;
    }

    if(new_child == NULL && child != NULL)
    {
        for(idx = 0; idx < last_index(curr->data_item); idx++)
        {
            size += sf_size_of_tree(child[idx]);
        }
    }
    while(new_child)
    {
            size += sf_size_of_tree(new_child);
            new_child = new_child->next;
    }
    size = size/(1024*1024);
    size += sf_size_of_tree(curr->fptree ? curr->fptree->root : NULL);
    return size;
}


long unsigned sf_no_of_nodes(sfnode curr)
{
    if(curr == NULL)
    {
        return 0;
    }
    long unsigned sum = 1;
    int i;
    for(i = 0; i < last_index(curr->data_item); i++)
    {
        // printf("sum = %lu\n");
        sum += sf_no_of_nodes(curr->children[i]);
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
    new_node->child = NULL;
    new_node->next = NULL;
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
void sf_insert_new_child(sfnode current_node, sfnode new_child, int d)
{
    new_child->parent = current_node;
    sfnode temp = current_node->child;
    new_child->next = temp;
    current_node->child = new_child;
}


sfnode sf_copy_node(sfnode current_node, data_type d)
{
    sfnode new_node = calloc(1, sizeof(struct sf_node));
    // we have pointer of children and children themselves
    d < 0 ? new_node->children = NULL : (new_node->children = calloc(last_index(d), sizeof(sfnode)));
    // new_node->item_list = calloc(last_index(d->data_item), sizeof(data));
    new_node->data_item = current_node->data_item;
    new_node->parent = current_node->parent;
    new_node->child = current_node->child;
    new_node->next = current_node->next;
    new_node->ftid = current_node->ftid;
    new_node->ltid = current_node->ltid;
    return new_node;
}
/****************************************************************************/

int sf_no_children(sfnode current_node)
{
    sfnode* current_child_ptr = current_node->children;
    sfnode child = current_node->child;
    int idx, no_children = 0;
    if(child == NULL && current_child_ptr)
    {
        for(idx = 0; idx < last_index(current_node->data_item); idx++)
        {
            if(current_child_ptr[idx])
                no_children++;
        }
    }

    else if(child != NULL)
    {
        while(child != NULL)
        {
            no_children++;
            child = child->next;
        }
    }

    return no_children;
}


int sf_no_dataitem(sfnode current_node)
{
    // data* current_data_ptr = current_node->item_list;
    data * current_data_ptr = NULL;
    sfnode child = current_node->child;
    int idx, no_dataitem = 0;

    if(child == NULL && current_data_ptr)
    {
        for(idx = 0; idx < last_index(current_node->data_item); idx++)
        {
            if(current_data_ptr[idx])
                no_dataitem++;
        }
    }

    else if(child != NULL)
    {
        while(child != NULL)
        {
            no_dataitem++;
            child = child->next;
        }
    }

    return no_dataitem;
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
    buffer last = curr->buffertail, temp_buff = curr->bufferhead;

    while(temp_buff)
    {
        temp_buff->freq *= pow(DECAY, tid - temp_buff->ltid);
        temp_buff->ltid = tid;
        if(sf_is_equal(temp_buff->itemset, d))
        {
            // printf("appending at node!\n");
            // sf_print_node(curr);
            temp_buff->freq++;
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
    data temp;
    new->ftid = tid;
    new->ltid = tid;
    new->itemset = (data) calloc(1, sizeof(struct data_node));
    new->itemset->data_item = d->data_item;
    new->freq = 1;
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
    {
        last->next = new;
        new->prev = last;
    }

    curr->buffertail = new;
    curr->buffertail->next = NULL;
    curr->bufferSize++;
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


void sf_fp_insert(sfnode current_node, header_table* htable, data d, double cnt, int tid)
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

    if(d == NULL)    return;    //terminate when all items have been inserted

    // iterate through children
    // if the next data item has already occurred earlier, go along that child
    // otherwise make a new child
    sfnode current_child_ptr = current_node->child, prev = NULL, temp = NULL;
    // data current_data_ptr = current_node->item_list;
    idx = index(d->data_item, current_node->data_item);

    while(current_child_ptr && current_child_ptr->data_item != d->data_item)
    {
        prev = current_child_ptr;
        current_child_ptr = current_child_ptr->next;
    }

    if(current_child_ptr == NULL)
    {
        // copying the functionality of 'sf_create_and_insert_new_child(current_node, d, tid)'
        sfnode new_node = calloc(1, sizeof(struct sf_node));
        // printf("created node: %d\n", d->data_item);
        temp = current_node->child;
        new_node->next = temp;
        current_node->child = new_node;
        new_node->parent = current_node;
        new_node->data_item = d->data_item;
        new_node->ftid = tid;
        new_node->ltid = tid;
        current_child_ptr = new_node;
    }
    // assert(current_child_ptr[idx] != NULL);

    // printf("could not find appropriate child for %d:(\n", current_node->data_item);
    // data this_data_item = current_data_ptr;
    sf_fp_insert(current_child_ptr, htable, d->next, cnt, tid);
    return;
}


void sf_insert_itemset_helper(sfnode node, header_table* htable, int tid)
{
    /* currently node is the root node*/
    assert(node != NULL);

    // sf_print_buffer(current_node);
    QStack* qstack = createQStack(); /* initialize a qstack*/
    push(qstack, node); /* push the root node in the qstack*/
    sfnode current_node, this_child;
    sfnode* current_child_ptr;
    data temp, d;

    int idx, root_data, no_trans = 0;

    while(qstack->size > 0) /* we go on till the time we have nodes in the stack*/
    {
        current_node = get(qstack); /* get the node in LIFO manner ie. queue.
                                       This is to get the nodes level by level*/
        assert(current_node != NULL); /* since qstack is not empty, fetched node cant be null*/

        data_type this_data = current_node->data_item;
        root_data = htable[0]->data_item;
        idx = index(this_data, root_data); /* this index will point to the correct header node and the child node*/

        sf_create_update_header_node(htable, this_data, root_data, tid); /* create the header node if it's not made yet*/

        /* create the links right here only*/
        if(htable && current_node->hnode == NULL)
        {
            //append to the head of the linked list for this data item
            current_node->next_similar = htable[idx]->first;
            current_node->hnode = htable[idx];
            if(htable[idx]->first)
                htable[idx]->first->prev_similar = current_node;

            htable[idx]->first = current_node;
            /*The first node next to the header table has NO previous node although a
            next node from the header table points to it*/
            current_node->prev_similar = NULL;
            // htable[idx]->cnt += node->freq;
        }

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
            // sf_print_data_node(d);
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
            current_node->fptree = sf_create_sftree(DICT_SIZE + 1);
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
            temp = popped->itemset;

            /* this code readies the buffer for children of current_node*/
            while(temp)
            {
                idx = index(temp->data_item, current_node->data_item); /* idx has the correct indices of
                                                                          the children of current_node*/

                /* this code just creates the nodes*/
                if(current_child_ptr[idx] == NULL)
                {
                    /*data item has to be inserted as new child*/
                    assert(current_child_ptr[idx] == NULL);
                    // assert(current_data_ptr[idx] == NULL);
                    sf_create_and_insert_new_child(current_node, temp, tid);
                }

                assert(current_node->children[idx] != NULL); /* we just updated that child!*/

                this_child = current_child_ptr[idx];
                // data this_data_item = current_data_ptr[idx];
                // assert(is_equal(this_data_item, temp));

                if(temp->next) /* here we are filling up the buffers of the children*/
                    sf_append_buffer(current_child_ptr[idx], temp->next, tid);

                /* updating the frequency of the node according to the formula*/
                current_child_ptr[idx]->freq *= pow(DECAY, tid - current_child_ptr[idx]->ltid);
                assert(popped->ltid == tid); /* updated just after it was retrieved*/
                current_child_ptr[idx]->freq += popped->freq;
                current_child_ptr[idx]->ltid = tid;

                /*  this code is to update the header tables properly
                    note that now we dont need the update function
                */
                sf_create_update_header_node(htable, temp->data_item, root_data, tid);
                idx = index(temp->data_item, root_data); /* idx in the header table,\
                                                            root is the value of the root of sf-tree*/

                /* create the links right here only*/
                if(htable && this_child->hnode == NULL)
                {
                    //append to the head of the linked list for this data item
                    assert(htable[idx]->data_item == this_child->data_item);
                    this_child->next_similar = htable[idx]->first;
                    this_child->hnode = htable[idx];
                    if(htable[idx]->first)
                        htable[idx]->first->prev_similar = this_child;

                    htable[idx]->first = this_child;
                    /*The first node next to the header table has NO previous node although a
                    next node from the header table points to it*/
                    this_child->prev_similar = NULL;
                    // htable[idx]->cnt += node->freq;
                }

                temp = temp->next; /* move the buffer node forward*/
            }

            /* pushing the children in the qstack so that the above procedure can be applied on them*/
            temp = popped->itemset;

            while(temp->next) /* we wont go all the way upto the last node as it has already been updated*/
            {
                idx = index(temp->data_item, current_node->data_item);
                sf_prune_buffer(current_child_ptr[idx], tid);
                temp = temp->next;
                if(current_child_ptr[idx]->freq > EPS*(tid - current_child_ptr[idx]->ftid))
                {
                    // printf("not pruning freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
                    current_child_ptr[idx]->ltid = tid;
                    double toss = ((double) rand())/RAND_MAX;
                    if((toss < CARRY && current_child_ptr[idx]->fptree == NULL) || CARRY == 2.0)
                    /* this is to ensure that the itemset is inserted when we are emptying the buffer.
                    Propagate the node downwards with a certain probability
                    this reduces the length of qstack and makes insertion faster
                    but we'll need to empty the nodes later on*/
                    {
                        push(qstack, current_child_ptr[idx]);
                    }
                }
                else
                {
                    // printf("freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
                    sf_delete_tree_structure(current_child_ptr[idx]);
                    current_child_ptr[idx] = NULL;
                }
            }
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
    while(d) /* this is taking time as with higher avg. len we have to insert in many trees*/
    {
        sftree tree = forest[d->data_item];
        if(d->next == NULL) /* d is a single item*/
        {
            tree->root->freq *= pow(DECAY, tid - tree->root->ltid);
            tree->root->ftid = min(tree->root->ftid, tid);
            tree->root->ltid = tid;
            tree->root->freq++;
            tree->head_table[index(d->data_item, tree->head_table[0]->data_item)]->cnt++;
            return;
        }
        sf_append_buffer(tree->root, d->next, tid); /* transaction: acdef, node a will have 'cdef'*/

        tree->root->ftid = min(tree->root->ftid, tid);
        tree->root->freq *= pow(DECAY, tid - tree->root->ltid);
        tree->root->freq++;
        tree->root->ltid = tid;

        sf_insert_itemset_helper(tree->root, tree->head_table, tid);
        d = d->next;
    }
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

    double minsup = pattern>0 ? (pattern == 2 ? SUP : MINSUP_FREQ) : MINSUP_SEMIFREQ;
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
    while(buff)
    {
        int t = 0, len = 0, new_end = end;

        while(buff && buff->ltid < minsup)
            buff = buff->next;

        if(buff == NULL)
            break;

        temp = buff->itemset;
        sf_sort_data(buff->itemset, NULL);

        while(temp)
        {
            len++;
            temp = temp->next;
        }

        /* this is to prevent the cases when the last transaction in collected matches the
           first transaction in temp*/
        temp = buff->itemset;
        if(temp->data_item == collected[end])
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

        while(temp)
        {
            fprintf(sf, " %d", temp->data_item);
            temp = temp->next;
        }

        // fprintf(sf, "  cnt = %d\n", cnt < 0 ? buff->tid : min(cnt, buff->tid));

        if(pattern%2 == 0)
        {
            fprintf(sf, " %lf\n", cnt < 0 ? buff->ltid : min(cnt, buff->ltid));
            // printf(" %lf\n", node->freq);
        }
        buff = buff->next;
    }

    // fprintf(sf, "\n");
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
            // printf("\nprinting fptree at node\n", end);
            // sf_print_node(node);
            /* there is an FPTree at this node*/
            sfnode collect_node = calloc(1, sizeof(struct sf_node));
            if(node->fptree != NULL)
            {
                data sorted = sf_create_sorted_dummy(node->fptree->root->data_item + 1);
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
                // sf_print_buffer(collect_node);
                sf_delete_data_node(sorted);
            }
            cnt += sf_print_patterns_to_file(collected, collect_node->bufferhead, node->freq, end, pattern);
            // sf_delete_buffer(collect_node->bufferhead);
            sf_delete_tree_structure(collect_node);
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
    printf("mining the tree with support: %lf\n", N*minsup);

    for(idx = 0; idx < DICT_SIZE; idx++)
    {
        sfnode root = forest[idx]->root;
        // printf("root = %d, freq = %lf\n", root->data_item, root->freq);
        cnt += sf_mine_frequent_itemsets_helper(root, collected, -1, tid, pattern);
    }
    free(collected);
    return cnt;
}


sfnode sf_dfs(sfnode node, header_table* htable, data_type highest_priority_data_item)
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
    sfnode new_node = calloc(1, sizeof(struct sf_node));

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

    sfnode temp_child_list = node->child;
    // data* temp_data = node->item_list;
    int last = node->data_item >= 0 ? last_index(node->data_item) : last_index(root_data);

    while(temp_child_list)
    {
        // printf("adding child!\n");
        sfnode this_child = temp_child_list;
        if(this_child->touched > 0)
        {
            sfnode new_child = sf_dfs(this_child, htable, highest_priority_data_item);
            if(new_child == NULL)    continue;
            sf_insert_new_child(new_node, new_child, this_child->data_item);
        }
        temp_child_list = temp_child_list->next;
    }

    node->touched = 0;
    return new_node;
}


sftree sf_create_conditional_sf_tree(sftree tree, data_type data_item, double minsup, int tid)
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

    sfnode node = curr_head_table_node->first;
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
    sfnode temp;
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

    sftree cond_tree = sf_create_sftree(DICT_SIZE + 1); /* this is because in FP-Trees we don't need the children array*/
    cond_tree->root->data_item = tree->head_table[0]->data_item;
    cond_tree->head_table = NULL;
    sf_create_header_table(cond_tree, tid);
    free(cond_tree->root); /* as we are assigning a custom root later*/

    /* now run a DFS from the root of the given FP_tree, for all touched nodes,*/
    /* create a copy for the conditional FP-tree*/
    sfnode cond_sftree = sf_dfs(tree->root, cond_tree->head_table, data_item);
    if(cond_sftree == NULL)
    {
        // printf("condtree = NULL!!!\n");
        return NULL;
    }

    // printf("cond_sftree, highers_prty_item = %d\n", data_item);
    // sf_print_tree(cond_sftree);
    // printf("\n");

    cond_tree->root = cond_sftree;
    // printf("child data = %d\n", cond_sftree->child?cond_sftree->child->data_item:0);
    // sf_print_header_table(cond_tree->head_table);
    return cond_tree;
}


void sf_fp_mine_frequent_itemsets(sftree tree, data sorted, data till_now, sfnode collected, int tid, double minsup)
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

    if(till_now != NULL)
    {
        // printf("till_now = ");
        data temp = till_now;
        // sf_print_data_node(till_now);

        //current itemset is not empty
        data last_item = till_now;

        while(last_item->next != NULL)
            last_item = last_item->next;

        /* get the proper index*/
        idx = index(last_item->data_item, tree->head_table[0]->data_item);

        curr_header_node = tree->head_table[idx];
        if(tree->head_table[idx] == NULL)
        {
            tree->head_table[idx] = calloc(1, sizeof(struct header_table_node));
            tree->head_table[idx]->data_item = last_item->data_item;
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
            // sf_print_data_node(till_now);
            sf_append_buffer(collected, till_now, curr_header_node->cnt);
            // printf("buffer of collected node in fp_mine: ");
            // sf_print_buffer(collected);
        }
    }

    if(sorted == NULL)    return;

    //now check for supersets of this itemset by considering every next data item
    //in the sorted list
    data curr_data = sorted;
    while(curr_data != NULL)
    {
        sftree cond_tree = sf_create_conditional_sf_tree(tree, curr_data->data_item,
                           minsup, tid);

        if(cond_tree == NULL)
        {
            // printf("skipped %d\n", curr_data->data_item);
            curr_data = curr_data->next;
            continue;
        }

        //append to front of current itemset
        data new_data = calloc(1, sizeof(struct data_node));
        new_data->data_item = curr_data->data_item;
        new_data->next = NULL;

        if(till_now == NULL)
            till_now = new_data;

        else
        {
            data temp = till_now;
            while(temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = new_data;
        }

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

        sf_fp_mine_frequent_itemsets(cond_tree, curr_data->next, till_now, collected, tid, minsup);
        sf_delete_sftree(cond_tree);
        // if(curr_data->next != NULL)
        //     printf("finished mining %d\n", curr_data->next->data_item);
        // else
        //     printf("finished mining NULL\n");

        // printf("deleted %d from till_now\n", curr_data->data_item);

        //delete from front of current itemset
        if(till_now->next == NULL)
        {
            free(till_now);
            till_now = NULL;
        }

        else
        {
            data temp_del = till_now->next;
            data prev = till_now;
            while(temp_del->next != NULL)
            {
                temp_del = temp_del->next;
                prev = prev->next;
            }
            prev->next = NULL;
            free(temp_del);
        }
        curr_data = curr_data->next;
    }
}


/****************************************************************************/

/* this function fixes the next_similar variable ie. includes the new nodes in the next_similar scheme*/
void sf_create_header_table_helper(sfnode root, header_table* h)
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

    sfnode current_child_ptr = root->child;
    while(current_child_ptr)
    {
        sf_create_header_table_helper(current_child_ptr, h);
        current_child_ptr = current_child_ptr->next;
    }
}


/*  this function is updating the header table of the sftree... NOT NEEDED*/
void sf_update_header_table(sftree tree, int tid)
{
    header_table* htable = tree->head_table;
    int idx, root_data = htable[0]->data_item;
    for(idx = 0; idx < last_index(root_data); idx++)
    {
        if(htable[idx])
        {
            htable[idx] = NULL;
        }
    }
}


void sf_create_header_table(sftree tree, int tid)
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
    // sf_create_header_table_helper(tree->root, tree->head_table);

    // data sorted = sf_create_sorted_dummy(tree->root->data_item);
    // sf_update_header_table(tree->head_table, sorted, tid);
    // sf_delete_data_node(sorted);
    // free(sorted);
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


void sf_sort_data(data head, double* arr)
{
    // removes duplicate items also
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
        root = forest[i]->root;
        htable = forest[i]->head_table;
        push(qstack, root);
        if(root->bufferhead != NULL) /* push the buffered itemsets down*/
        {
            sf_insert_itemset_helper(root, htable, tid);
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
                        sf_insert_itemset_helper(current_node->children[idx], htable, tid);
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


void sf_fp_merge1(sfnode parent, sfnode child, int tid)
{

    // child's parent has been detached
    // assert(child->parent->parent == NULL);
    child->parent = parent;
    child->children = NULL;
    if(parent->children == NULL)
    {
        // assert(parent->item_list == NULL);
        // parent->child = calloc(1, sizeof(struct sf_node)); /* copying the node*/
        parent->child = child;
        return;
    }

    sfnode childptr = parent->child, prev = NULL;
    header_table htemp;
    // data curr_item = parent->item_list, prev_item = NULL;
    data_type dat = child->data_item;

    while(childptr && childptr->data_item != dat)
    {
        prev = childptr;
        // prev_item = curr_item;
        // assert(prev->data_item == prev_item->data_item);
        childptr = childptr->next;
        // curr_item = curr_item->next;
    }

    if(childptr == NULL)
    {
        // assert(curr_item == NULL);
        // prev->next = (sfnode) calloc(1, sizeof(struct sf_node));
        prev->next = child;
        prev->next->next = NULL;
        child->parent = parent;

        // prev_item->next = (data) calloc(1, sizeof(struct data_node));
        // prev_item->next->data_item = child->data_item;
        // prev_item->next->next = NULL;
        return;
    }

    /* this means that there is a node in the children of the parent which has the same data_item as child*/
    assert(childptr != NULL);
    // assert(curr_item != NULL);
    // assert(childptr->data_item == curr_item->data_item);

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

    sfnode child_child = child->child;
    child->child = NULL;
    while(child_child)
    {
        // child_child->tree_node->parent = NULL;
        sf_fp_merge1(childptr, child_child, tid);
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
    sfnode fir = htemp->first, temp2, parent;
    // htemp->first = NULL;
    sfnode temp, ori, temp1;
    data prntdata, tmpdata;
    // if(fir)
    // {
    //     printf("\nNEW PRUNE CALL-- children of fir = %d:", fir->data_item);
    //     temp = fir->child;
    //     while(temp)
    //     {
    //         printf("-->%d", temp->data_item);
    //         temp = temp->next;
    //     }
    // }
    //  fir->parent ? sf_print_tree(fir->parent) : sf_print_tree(fir);
    int root_data = htable[0]->data_item;

    while(fir != NULL)
    {
        parent = fir->parent;
        if(fir->data_item == root_data)
        {
            return 1;
        }
        parent->children = NULL;
        // printf("\nin pruing parent %d, children = %d: ", parent->data_item, sf_no_children(parent));
        if(parent->child == NULL)
            parent->child = fir;
        assert(parent->child != NULL);
        // fir->parent = NULL;
        // fp_print_data_node(parent->item_list);

        /* this code is separating the child from the parent and then we will merge\
           the parent and it's grandchildren*/
        // printf("fir(%d) is a child of parent(%d)!\n", fir->data_item, parent->data_item);
        // sf_print_tree(fir);
        if(parent->child == fir)
        {
            // printf("\nfir(%d) is first child of parent(%d)!\n", fir->data_item, parent->data_item);
            temp = parent->child;
            // tmpdata = parent->item_list;
            parent->child = parent->child->next;
            // parent->item_list = parent->item_list->next;
            // assert(temp == fir);
            // free(temp);
            // free(tmpdata);
        }
        else
        {
            temp = parent->child;
            // prntdata = parent->item_list;
            ori = NULL;
            while(temp->next != NULL)
            {
                if(temp->next == fir)
                {
                    // ori is the child pointer which points to fir, so we must free it to save space
                    ori = temp->next;
                    // tmpdata = prntdata->next;

                    temp->next = temp->next->next;
                    // prntdata->next = prntdata->next->next;

                    // free(ori);
                    // free(tmpdata);
                    break;
                }
                temp = temp->next;
                // prntdata = prntdata->next;
            }
        }
        /* temp holds a pointer to fir*/
        // printf("separated child = %d with %d children: ", fir->data_item, sf_no_children(fir));
        // sf_print_data_node(fir->item_list);

        temp1 = fir->child;

        while(temp1 != NULL)
        {
            // if(fir)
            // {
            //     printf("\nchildren of fir b4 merging = %d:", fir->data_item);
            //     temp = fir->child;
            //     while(temp)
            //     {
            //         printf("-->%d", temp->data_item);
            //         temp = temp->next;
            //     }
            // }

            // printf("\nmerging %d, %d\n", fir->parent->data_item, temp1->data_item);
            // printf("before:- parent %d, children = %d, items = %d: \n\n", parent->data_item,\
            //      sf_no_children(parent), sf_no_dataitem(parent));

            // if(parent->child)
            // {
            //     printf("\nchildren of parent merging = %d:", parent->data_item);
            //     temp = parent->child;
            //     while(temp)
            //     {
            //         printf("-->%d", temp->data_item);
            //         temp = temp->next;
            //     }
            // }

            // fp_print_data_node(parent->item_list);
            // sf_print_tree(parent);
            // assert(fp_verify_node(parent));

            sf_fp_merge1(parent, temp1, tid);

            //assert(fp_verify_node(parent));

            // printf("\nafter:- parent %d, children = %d, items = %d: \n\n", parent->data_item,\
            //      sf_no_children(parent), sf_no_dataitem(parent));

            // // fp_print_data_node(parent->item_list);
            // sf_print_tree(parent);

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

        // fp_delete_data_node(temp2->item_list);
        // assert(temp2->children == NULL);
        // free(temp2);
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

    /* delete the head if it needs to be deleted*/
    while(curr_buff == head && head != NULL)
    {
        idx = index(curr_buff->itemset->data_item, root_data);
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
        idx = index(curr_buff->itemset->data_item, root_data);
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
}

/* Decay the count of node in the header table and decrease some value from nodes in the bltree*/
void sf_prune_helper(sfnode node, header_table* htable, int tid)
{
    node->freq *= pow(DECAY, tid - node->ltid);
    int child, idx, root_data = htable[0]->data_item, i;
    sfnode first, next, curr;
    QStack* qstack = createQStack();

    // for(idx = 0; idx < last_index(root_data) && htable[idx]; idx++)
    // {
    //     if(htable[idx]->cnt < EPS*(tid - htable[idx]->ftid))
    //     {
    //         first = htable[idx]->first;
    //         while(first)
    //         {
    //             next = first->next_similar;
    //             first->prev_similar = NULL;
    //             first->parent->children[index(first->data_item, root_data)] = NULL;
    //             sf_delete_tree_structure(first);
    //             first = next;
    //         }
    //     }

    //     else
    //     {
    //         first = htable[idx]->first;
    //         while(first)
    //         {
    //             next = first->next_similar;
    //             if(first->freq < EPS*(tid - first->ftid))
    //             {
    //                 next->prev_similar = first->prev_similar;
    //                 if(first->prev_similar == NULL)
    //                     htable[idx]->first = next;
    //                 else
    //                     first->prev_similar->next_similar = next;

    //                 first->parent->children[index(first->data_item, root_data)] = NULL;
    //                 sf_delete_tree_structure(first);
    //             }
    //             first = next;
    //         }
    //     }
    // }

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
                    sf_delete_tree_structure(node->children[child]);
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
                                    sf_delete_sftree(node->children[child]->fptree);
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
}


void sf_prune(sforest forest, int tid)
{
    int i;
    for(i = 0; i < DICT_SIZE; i++)
    {
        sf_prune_helper(forest[i]->root, forest[i]->head_table, tid);
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

    if(node->fptree)
    {
        printf("FPTREE:\n");
        sf_print_tree(node->fptree->root);
        printf("FP-HEADER TABLE:-\n");
        printf("##############################################################################\n");
        sf_print_header_table(node->fptree->head_table);
        printf("##############################################################################\n");
        printf("******************************************************************************\n");
    }

    // printf("BUFFER:\n");
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
        printf("HEADER TABLE:-\n");
        sf_print_header_table(forest[i]->head_table);
        printf("***************************************\n");
        sf_print_tree(forest[i]->root);
        printf("---------------------------------------\n\n");
    }
}


void sf_print_sforest_lvl(sforest forest)
{
    sftree tree;
    int i, idx, root_data;
    for(i = 0; i < DICT_SIZE; i++)
    {
        tree = forest[i];
        printf("Printing SF-tree %d\n", tree->root->data_item);
        printf("HEADER TABLE:-\n");
        sf_print_header_table(tree->head_table);
        printf("######################################################################\n");

        header_table* htable = tree->head_table;
        root_data = htable[0]->data_item;
        for(idx = 0; idx < last_index(root_data) && htable[idx]; idx++)
        {
            printf("Printing NODE %d\n", htable[idx]->data_item);
            sfnode node = htable[idx]->first;
            while(node)
            {
                sf_print_node(node);
                node = node->next_similar;
            }
            printf("******************************************************************\n");
        }
        printf("\n");
    }
}


void sf_print_buffer(buffer head)
{
    buffer buff = head;
    while(buff)
    {
        printf("tid: %d --> ", buff->ltid);
        sf_print_data_node(buff->itemset);
        buff = buff->next;
    }
}


void sf_print_tree(sfnode node)
{
    sf_print_node(node);
    sfnode* curr_child_list = node->children;
    sfnode temp = node->child;
    int idx;
    if(node->child == NULL && curr_child_list)
    {
        for(idx = 0; idx < last_index(node->data_item); idx++)
        {
            if(curr_child_list[idx] != NULL)
                sf_print_tree(curr_child_list[idx]);
            // printf("going to child %d %d\n", this_child->data_item, curr_data->data_item);
        }
    }

    else if(node->child)
    {
        while(temp)
        {
            sf_print_tree(temp);
            temp = temp->next;
        }
    }
}


void sf_print_header_table(header_table* h)
{
    int idx;
    // z is the size of the table
    for(idx = 0; idx < last_index(h[0]->data_item) && h[idx] != NULL; idx++)
    {
        printf("item = %d, ftid = %d, ltid = %d, cnt = %lf\n", h[idx]->data_item, h[idx]->ftid, h[idx]->ltid, h[idx]->cnt);
        sfnode node = h[idx]->first;
        // while(node != NULL && node->next_similar != node)
        // {
        //     sf_print_node(node);
        //     node = node->next_similar;
        //     z++;
        // }
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