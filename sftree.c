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
10. Implementing lazy buffer allocation
11. If some node's buffer goes fully empty, don't delete the hbuffer field; we'll use ot later
12. Need to take down buffered transactions in a bunch now
*/
#include "sftree.h"


/* ASCENDING order here*/
int sf_cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}


int sf_is_equal(data d1, data d2)
{
    if(d1[1] != d2[1])
        return 0;
    int i;
    for(i = 0; i < d1[1]; i++)
    {
        if(d1[d1[0] + i + 2] != d2[d2[0] + i + 2])
            return 0;
    }
    return 1;
}


// Initialiazing an empty forest
sforest sf_create_sforest() // seen
{
    int i;
    sforest forest = (sforest) calloc(DICT_SIZE, sizeof(sfnode)); // forest an array
    for(i = 0; i < DICT_SIZE; i++)
    {
        forest[i] = sf_create_sfnode(i); // creating a root node for each tree in the forest in a smart way.
    }
    return forest;
}

// Smart initialization of root node of a BL-tree
sfnode sf_create_sfnode(data_type dat) // seen
{
    sfnode node = calloc(1, sizeof(struct sf_node));

    if(node == NULL)
    {
        printf("node calloc failed\n");
    }

    node->children = calloc(last_index(dat), sizeof(sfnode)); // smart allocation of children array using a macro
    node->hbuffer = calloc(HSIZE, sizeof(bufferTable));
    node->data_item = dat;
    node->ftid = INT_MAX;
    node->ltid = -1;
    return node;
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


void sf_delete_sftree_structure(sfnode current_node)
{
    if(current_node == NULL)
        return;
    sfnode* current_child_ptr = current_node->children;
    sfnode this_child, temp;
    int idx;
    sf_delete_buffer_table(current_node->hbuffer);
    free(current_node->hbuffer);
    // bufftable = NULL;
    current_node->hbuffer = NULL;

    sf_delete_fptree(current_node->fptree);
    current_node->fptree = NULL;

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
                current_child_ptr[idx] = NULL;
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


void sf_delete_data_node(data d) // seen
{
    // deletes the linked list of arriving transactions, after they are inserted.
    free(d);
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
        sf_delete_data_node(head->itemset);
        free(head);
        head = NULL;
        head = next;
    }
}


void sf_delete_buffer_table(bufferTable* bufftable)
{
    int idx;
    if(bufftable)
    {
        for(idx = 0; idx < HSIZE; idx++)
        {
            if(bufftable[idx])
            {
                sf_delete_buffer(bufftable[idx]->bufferhead); /* clear up the buffer*/
                free(bufftable[idx]);
                bufftable[idx] = NULL;
            }
        }
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


void sf_delete_fptree(fptree tree)
{
    if(tree == NULL)    return;
    sf_delete_header_table(tree->head_table);
    sf_delete_fptree_structure(tree->root);
    free(tree->root);
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
        free(forest[idx]);
    }
    return;
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
void sf_create_and_insert_new_child(sfnode current_node, data_type d, int tid)
{
    sfnode new_node = calloc(1, sizeof(struct sf_node));
    // we have pointer of children and children themselves
    new_node->children = calloc(last_index(d), sizeof(sfnode));
    new_node->data_item = d;
    new_node->parent = current_node;
    new_node->ftid = tid;
    new_node->ltid = tid;
    new_node->hbuffer = calloc(HSIZE, sizeof(bufferTable));

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


data sf_copy_data(data d)
{
    data temp = malloc((d[1]+2)*sizeof(int));
    memcpy(temp, d + first(d) - 2, (d[1] + 2)*sizeof(int)); // copy from starting position
    temp[0] = 0;
    temp[1] = d[1];
    return temp;
}
/****************************************************************************/

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


void sf_get_least_ftid(sfnode node)
{
    if(node == NULL)
        return;
    LEAVE_AS_BUFFER = min(LEAVE_AS_BUFFER, node->ftid);
    int i;
    for(i = 0; i < last_index(node->data_item); i++)
    {
        sf_get_least_ftid(node->children[i]);
    }
}


void sf_check_node_buffer(sfnode curr)
{
    int cnt_buffer = 0, idx, cnt_table = 0;
    for(idx = 0; idx < HSIZE; idx++)
    {
        if(curr->hbuffer[idx])
        {
            cnt_table++;
            if(curr->hbuffer[idx]->bufferhead)
            {
                // sf_print_buffer(curr->hbuffer[idx]->bufferhead);
                cnt_buffer++;
            }
        }
    }
    if(cnt_buffer == 0)
        assert(curr->bufferSize == 0);
    else
        assert(curr->bufferSize > 0);
}


void sf_append_buffer(sfnode curr, data d, double freq, int tid) // seen
{
    /* takes care of cases when to be buffered item is NULL*/
    /* this tid is the timestamp when the dataitem was buffered*/
    if(d[1] == 0)
        return;

    // printf("appending to buffer of node\n");
    // sf_print_node(curr);
    // sf_print_data_node(d);

    int bucket = d[first(d)] % HSIZE; // hash according to the first element

    if(curr->hbuffer[bucket] == NULL)
    {
        // printf("d->data_item = %d, mod HSIZE = %d\n", d->data_item, bucket);
        curr->hbuffer[bucket] = calloc(1, sizeof(buffer_table));
    }
    // bufferTable is an element of the hash table. There will DLL below buffer table.
    bufferTable curr_buffer = curr->hbuffer[bucket];

    if(bucket != d[first(d)]) // comparing with bucket id. Eg. Bucket 3 can have transaction starting from 23.
        curr_buffer->collision = '1';

    buffer last = curr_buffer->buffertail;
    buffer head = curr_buffer->bufferhead;
    curr->last = bucket; // integer representing the index of the buffer table from\
                            which we have to remove an item when its turn comes back.

    /* updating the count of bufferTable*/
    // curr_buffer->freq *= pow(DECAY, tid - curr_buffer->ltid);
    // curr_buffer->freq += freq; /* this is needed as when while insertion we are bringing the accumulated
                                  // transactions down then we need to maintain their count*/
    // curr_buffer->ltid = tid;

    while(head)
    {        
        if(sf_is_equal(head->itemset, d))
        {
            head->freq *= pow(DECAY, tid - head->ltid);
            head->ltid = tid;
            head->freq += freq;
            return;
        }
        head = head->next;
    }

    /* at this point it is clear that we have to create a new buffer node only*/
    buffer new = (buffer) calloc(1, sizeof(struct buffer_node));
    new->ftid = tid;
    new->ltid = tid;
    new->freq = freq;
    assert(head == NULL);


    /* bufferSize fields increase only when we add a new node*/
    curr->bufferSize++; // incrementing the count of number of transactions in the buffer

    /* we create a new copy of the datanode*/
    data temp;
    
    new->itemset = sf_copy_data(d);

    /* this means that the whole buffertable cell is empty, we're inserting for the first time*/
    if(curr_buffer->bufferhead == NULL)
    {
        curr_buffer->bufferhead = new;
        curr_buffer->buffertail = new;
        return;
    }

    if(curr_buffer->buffertail)
    {
        curr_buffer->buffertail->next = new;
    }

    curr_buffer->buffertail = new;
    curr->hbuffer[bucket] = curr_buffer; /* make sure that the final changes\
                                            are reflected the buffertable of the node also*/
    assert(curr->hbuffer[bucket]->bufferhead != NULL);
    return;
}


buffer sf_pop_buffer(sfnode curr, int bucket, int tid) //seen
{
    if(curr->bufferSize == 0 || curr->hbuffer == NULL)
    {
        return NULL;
    }

    if(curr->last != -1)
        bucket = curr->last;

    /* this should not get stuck forever as buffersize is not 0*/
    while(curr->hbuffer[bucket] == NULL || curr->hbuffer[bucket]->bufferhead == NULL)
    {
        bucket = (bucket+1)%HSIZE;
    }

    bufferTable curr_buffer = curr->hbuffer[bucket];
    buffer to_pop = curr_buffer->bufferhead;
    assert(to_pop != NULL);    
    to_pop = curr_buffer->bufferhead;
    curr_buffer->bufferhead = to_pop->next;
    // curr_buffer->freq *= pow(DECAY, tid - curr_buffer->ltid); /* update hbuffer's freq and ltid*/
    // curr_buffer->ltid = tid;

    /* updating the outgoing buffer here only*/
    to_pop->freq *= pow(DECAY, tid - to_pop->ltid);
    to_pop->ltid = tid;
    to_pop->next = NULL;

    // curr_buffer->freq -= to_pop->freq;

    curr->last = -1;

    if(curr_buffer->bufferhead == NULL)
    {
        assert(curr_buffer->buffertail == to_pop);
        curr_buffer->buffertail = NULL;
        // free(curr->hbuffer[bucket]);
        // curr->hbuffer[bucket] = NULL;
    }
    curr->hbuffer[bucket] = curr_buffer;
    curr->bufferSize--;
    // sf_check_node_buffer(curr);
    return to_pop;
}


void sf_fp_insert(fpnode current_node, header_table* htable, data d, double cnt, int tid)
{
    assert(current_node != NULL);
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

    if(d[1] == 0)    return;    //terminate when all items have been inserted

    // iterate through children
    // if the next data item has already occurred earlier, go along that child
    // otherwise make a new child
    fpnode current_child_ptr = current_node->child, prev = NULL, temp = NULL;
    // data current_data_ptr = current_node->item_list;
    idx = index(d[first(d)], current_node->data_item);

    while(current_child_ptr && current_child_ptr->data_item != d[d[0]])
    {
        prev = current_child_ptr;
        current_child_ptr = current_child_ptr->next;
    }

    if(current_child_ptr == NULL)
    {
        // copying the functionality of 'sf_create_and_insert_new_child(current_node, d, tid)'
        fpnode new_node = calloc(1, sizeof(struct fp_node));
        // printf("created node: %d\n", d->data_item);
        temp = current_node->child;
        new_node->next = temp;
        current_node->child = new_node;
        new_node->parent = current_node;
        new_node->data_item = d[first(d)];
        new_node->ftid = tid;
        new_node->ltid = tid;
        current_child_ptr = new_node;
    }
    // assert(current_child_ptr[idx] != NULL);

    // printf("could not find appropriate child for %d:(\n", current_node->data_item);
    // data this_data_item = current_data_ptr;
    d[0]++;
    d[1]--;
    sf_fp_insert(current_child_ptr, htable, d, cnt, tid);
    return;
}


int sf_insert_itemset_helper(sfnode node, int root_data, int tid, double total_time, timeval* start)
{
    // /* currently node is the root node*/
    assert(node != NULL);

    // sf_print_buffer(current_node);
    QStack* qstack = createQStack(); /* initialize a qstack*/
    push(qstack, node); /* push the root node in the qstack*/
    sfnode current_node, this_child;
    sfnode* current_child_ptr;
    data temp, d;
    double toss, elapsedTime;
    timeval curr;
    int idx;

    while(qstack->size > 0) /* we go on till the time we have nodes in the stack*/
    {
        if(start)
        {
            gettimeofday(&curr, NULL);
            elapsedTime = (curr.tv_sec - start->tv_sec) * 1000.0;
            elapsedTime += (curr.tv_usec - start->tv_usec) / 1000.0;
        }

        /* this controls pre-emption*/ // leaving because there is no time.
        if(start != NULL && elapsedTime > total_time)
        {
            /* inserted in LIFO order*/
            // printf("leaving in buffer at node %d: ", current_node->data_item);
            // sf_print_data_node(popped->itemset);
            LEAVE_AS_BUFFER = 1; // being used for counting premptive nodes.
            delete_qstack(qstack); // try re-using memory
            return 0;
        }

        current_node = pop(qstack); /* get the node in LIFO manner ie. queue.
                                       This is to get the nodes level by level*/
        // assert(current_node != NULL); /* since qstack is not empty, fetched node cant be null*/

        data_type this_data = current_node->data_item;
        // sf_check_node_buffer(current_node);

        buffer popped = sf_pop_buffer(current_node, rand()%HSIZE, tid); /* popped is the last buffer, it has the itemset
                                                        which will be propagated down*/
        // sf_check_node_buffer(current_node);

        if(popped)
            assert(popped->next == NULL); /* 'popped' is the buffer node*/
        else
            continue;

        if(current_node->bufferSize > 0) /* push the node back in the qstsack if it's buffer is still not empty*/
            push(qstack, current_node);

        // assert(popped->ltid == tid);

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
            temp = popped->itemset; // first item of the transaction

            /* this code readies the buffer for children of current_node*/
            while(temp[1] > 0)
            {
                idx = index(temp[first(temp)], current_node->data_item); /* idx has the correct indices of
                                                                          the children of current_node*/

                /* this code just creates the nodes*/
                if(current_child_ptr[idx] == NULL)
                {
                    /*data item has to be inserted as new child*/
                    sf_create_and_insert_new_child(current_node, temp[first(temp)], tid);
                }

                this_child = current_child_ptr[idx];
                assert(this_child->parent == current_node);

                if(temp[1] > 1) /* here we are filling up the buffers of the children*/
                {
                    temp[0]++;
                    temp[1]--;
                    sf_append_buffer(current_child_ptr[idx], temp, popped->freq, tid);
                    temp[0]--;
                    temp[1]++;
                }

                /* updating the frequency of the node according to the formula*/
                current_child_ptr[idx]->freq *= pow(DECAY, tid - current_child_ptr[idx]->ltid);
                assert(popped->ltid == tid); /* updated in the pop function itself*/
                current_child_ptr[idx]->freq += popped->freq;
                current_child_ptr[idx]->ftid = min(current_child_ptr[idx]->ftid, popped->ftid);
                current_child_ptr[idx]->ltid = tid;
    
                /* pushing the children in the qstack so that the above procedure can be applied on them*/
                if(temp[1] > 1)
                {
                    idx = index(temp[first(temp)], current_node->data_item);
                    int idx_next = index(temp[first(temp) + 1], current_child_ptr[idx]->data_item);
                    
                    /* prune the buffer after certain time*/
                    if(tid - current_child_ptr[idx]->last_pruned > BATCH/10)
                    {
                        current_child_ptr[idx]->last_pruned = tid;
                        sf_prune_buffer(current_child_ptr[idx], tid);
                    }

                    if(current_child_ptr[idx]->freq > EPS*(tid - current_child_ptr[idx]->ftid))
                    {
                        // printf("not pruning freq = %lf, pbound = %lf\n", current_child_ptr[idx]->freq, EPS*(tid - current_child_ptr[idx]->ftid));
                        current_child_ptr[idx]->ltid = tid;
                        double toss = ((double) rand())/RAND_MAX;
                        if((toss < CARRY && current_child_ptr[idx]->fptree == NULL) || CARRY == 2.0)
                        // if((current_child_ptr[idx]->freq > THETA*(tid - current_child_ptr[idx]->ftid)\
                        //     && current_child_ptr[idx]->fptree == NULL\
                        //     && current_child_ptr[idx]->children[idx_next]) || CARRY == 2.0)
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
                        sf_delete_sftree_structure(current_child_ptr[idx]);
                        // free(current_child_ptr[idx]);
                        current_child_ptr[idx] = NULL;
                    }                    
                }
                temp[0]++; /* move the buffer node forward*/
                temp[1]--; /* decrease the length*/
            }
        }
        /* free the popped buffer to save space*/
        sf_delete_buffer(popped);
        // free(popped);
    }

    delete_qstack(qstack);
    return 1;
}


void sf_insert_itemset(sforest forest, data d, int tid, double total_time, timeval* start) // seen
{
    int flag = 1;
    while(d[1] > 0) /* this is taking time as with higher avg. len we have to insert in many trees*/
    {
        sfnode root = forest[d[first(d)]];
        // printf("d[0] = %d, d[1] = %d, first(d) = %d\n", d[0], d[1], first(d));
        if(d[1] == 1) /* d is a single item*/
        {
            root->freq *= pow(DECAY, tid - root->ltid); // updating frequency according to decay factor.
            root->ftid = min(root->ftid, tid); // updating first seen tid.
            root->ltid = tid; // updating latest updated tid. tid is the current tid.
            root->freq++;
            return;
        }
        d[0]++; // moving the transaction forward
        d[1]--; // decreasing the size
        sf_append_buffer(root, d, 1, tid); /* transaction: acdef, node a will have 'cdef'. We will set the freuency to 1.*/
        // printf("printing buffer of node %d\n", root->data_item);
        // sf_print_buffer_table(root->hbuffer);
        root->ftid = min(root->ftid, tid); // updating first seen time stamp
        root->freq *= pow(DECAY, tid - root->ltid);
        root->freq++;
        root->ltid = tid;

        // double toss = ((double) rand())/RAND_MAX;
        // if(toss < CARRY || CARRY == 2.0)
        // if(flag)
        {
            flag = sf_insert_itemset_helper(root, root->data_item, tid, total_time, start);
        }
    }
    return;
}


/*************************************************************************************************/
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
    int pttrn_cnt = 0, i;

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
        int len = 0, new_end = end, t = 0;

        while(buff && buff->freq < minsup)
            buff = buff->next;

        if(buff == NULL)
            break;

        temp = buff->itemset;
        len = temp[1];

        /* this is to prevent the cases when the last transaction in collected matches the
           first transaction in temp*/
        temp = buff->itemset;
        if(temp[first(temp)] == collected[end])
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
        for(i = 0; i < len; i++)
            fprintf(sf, " %d", temp[i + 2]);

        // //fprintf(sf, "  cnt = %d\n", cnt < 0 ? buff->tid : min(cnt, buff->tid));

        if(pattern%2 == 0)
        {
            fprintf(sf, " %lf\n", cnt < 0 ? buff->freq : min(cnt, buff->freq));
            // fprintf(sf, "\n");
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
    int cnt = 0, i;
    double minsup = pattern > 0 ? (pattern == 2 ? SUP : MINSUP_FREQ) : MINSUP_SEMIFREQ;
    if(node->freq >= N*minsup)
    {
        collected[++end] = node->data_item;
        if(end >= 0)
        {
            // LEAVE_AS_BUFFER = min(LEAVE_AS_BUFFER, node->ftid);
            // printf("\nprinting fptree at node\n", end);
            // sf_print_node(node);
            /* there is an FPTree at this node*/
            bufferTable collector = calloc(1, sizeof(buffer_table));
            if(node->fptree != NULL)
            {
                // printf("\n****mining this FP-tree****\n");
                // sf_print_tree(node->fptree->root);
                // printf("****HEADER TABLE:****\n");
                // sf_print_header_table(node->fptree->head_table);

                data_type sorted = node->fptree->root->data_item + 1;
                /* change the root data to -1 for mining correctly*/
                node->fptree->root->data_item = -1;
                
                /* a temporary data structure to be used within fpgrowth algo.*/                
                data till_now = calloc(DICT_SIZE + 2, sizeof(data_type));
                till_now[0] = 0;
                till_now[1] = 0;

                sf_fp_mine_frequent_itemsets(node->fptree, sorted, till_now, collector,\
                                             tid, N*minsup);
                free(till_now);
                /* reset the old (correct) value of root node data*/
                node->fptree->root->data_item = node->fptree->head_table[0]->data_item;

                // int i;
                // printf("collected: ");
                // for(i = 0; i < end + 1; i++)
                //     printf("%d, ", collected[i]);
                // printf("\n****node->freq = %lf, collected from tree****\n", node->freq);
            }
            cnt += sf_print_patterns_to_file(collected, collector->bufferhead, node->freq, end, pattern);
            // sf_delete_buffer(collect_node->bufferhead);
            sf_delete_buffer(collector->bufferhead);
            free(collector);
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
    }
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


void sf_fp_mine_frequent_itemsets(fptree tree, data_type sorted, data till_now, bufferTable collected, int tid, double minsup)
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

    if(till_now[1] > 0)
    {
        // printf("till_now = ");
        data_type temp = till_now[first(till_now)];
        // sf_print_data_node(till_now);

        //current itemset is not empty
        data_type last_item = till_now[last(till_now)];

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
            // sf_print_data_node(till_now);
            LEAVE_AS_BUFFER = min(LEAVE_AS_BUFFER, curr_header_node->ftid);

            buffer new = calloc(1, sizeof(struct buffer_node));
            new->itemset = sf_copy_data(till_now);
            
            /* code for appending the buffer at the start*/
            buffer temp_head = collected->bufferhead;
            collected->bufferhead = new;
            new->next = temp_head;
            new->freq = curr_header_node->cnt;
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

        //append to end of till_now
        data_type new_data = curr_data;
        till_now[1]++;
        till_now[last(till_now)] = new_data;

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

        //delete from back of till_now
        // till_now[last(till_now)] = -1; // reset the last item
        till_now[1]--;
        curr_data++;
    }
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
}
/****************************************************************************/


data sf_sort_data(data d) // seen
{
    // does insertion sort of items and removes the duplicate items also
    qsort(d + 2, d[1], sizeof(d[0]), sf_cmpfunc);

    /*
       Removing duplicate items here in O(n)
    */
    int ori = d[2], prev_idx, temp, new_len = 1;

    for(prev_idx = 0, temp = prev_idx + 1; temp != d[1]; temp++)
    {
        while((temp < d[1] - 1) && d[temp + 2] == ori)
        {
            temp++;
        }

        if(temp < d[1])
        {
            d[prev_idx + 3] = d[temp + 2];
            ori = d[temp + 2];
            prev_idx++;
            new_len++;
        }
    }
    data new = malloc((new_len + 2) * sizeof(data_type));
    memcpy(new, d, (new_len + 2)*sizeof(data_type));
    new[0] = 0;
    new[1] = new_len;
    free(d);
    return new;
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
        root = forest[i];
        root_data = root->data_item;
        push(qstack, root);
        if(root->bufferSize > 0) /* push the buffered itemsets down*/
        {
            sf_insert_itemset_helper(root, root_data, tid, -1, NULL);
        }

        else
        {
            while(qstack->size > 0)
            {
                current_node = pop(qstack);
                last = last_index(current_node->data_item);
                for(idx = 0; idx < last; idx++)
                {
                    if(current_node->children[idx] && current_node->children[idx]->bufferSize > 0)
                    /* if buffer is not empty then simply
                       call the insert function*/
                    {
                        sf_insert_itemset_helper(current_node->children[idx], root_data, tid, -1, NULL);
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
    // assert(child->parent->parent == NULL);
    child->parent = parent;
    if(parent->child == NULL)
    {
        // assert(parent->item_list == NULL);
        // parent->child = calloc(1, sizeof(struct sf_node)); /* copying the node*/
        parent->child = child;
        return;
    }

    fpnode childptr = parent->child, prev = NULL;
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
        // prev->next = (fpnode) calloc(1, sizeof(struct sf_node));
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
        (child->prev_similar)->next_similar = child->next_similar;

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
    htemp->cnt = 0;
    htemp->ltid = -1;
    htemp->ftid = INT_MAX - 1;
    htemp->first = NULL;
    return 0;
}


void sf_prune_buffer(sfnode curr, int tid)
{
    int root_data = curr->data_item, idx, i;
    for(i = 0; i < HSIZE; i++) // iterating through all the buckets
    {
        /* short-cut pruning*/
        // if(curr->children[idx] == NULL && curr->hbuffer[i]->freq < EPS*(tid - curr_buff->ftid))
        if(curr->hbuffer[i])
        {
            buffer curr_buff = curr->hbuffer[i]->bufferhead, head = curr->hbuffer[i]->bufferhead, temp, prev = NULL;
            if(head == NULL)
                continue;

            /* delete the head if it needs to be deleted*/
            while(curr_buff == head && head != NULL)
            {
                idx = index(curr_buff->itemset[first(curr_buff->itemset)], root_data);
                curr_buff->freq *= pow(DECAY, tid - curr_buff->ltid);
                curr_buff->ltid = tid;
                if(curr->children[idx] == NULL && curr_buff->freq < EPS*(tid - curr_buff->ftid))
                {
                    temp = curr_buff;
                    curr_buff = curr_buff->next;
                    head = head->next;
                    temp->next = NULL;
                    curr->bufferSize--;
                    // curr->hbuffer[i]->freq -= temp->freq;
                    sf_delete_buffer(temp);
                }
                /* as we have already moved curr_buff to next node*/
                else
                    curr_buff = curr_buff->next;
            }
            curr->hbuffer[i]->bufferhead = head;
            if(head == NULL)
            {
                assert(curr_buff == NULL);
                curr->hbuffer[i]->buffertail = NULL;
                continue;
            }
            prev = head;
            /* B1<==>B2<==>[to_del]<==>B4<==>B5*/
            /* now we are beyond the point where heads need to be deleted*/
            while(curr_buff)
            {
                idx = index(curr_buff->itemset[first(curr_buff->itemset)], root_data);
                curr_buff->freq *= pow(DECAY, tid - curr_buff->ltid);
                curr_buff->ltid = tid;
                if(curr->children[idx] == NULL && curr_buff->freq < EPS*(tid - curr_buff->ftid))
                {
                    prev->next = curr_buff->next;
                    temp = curr_buff;
                    if(curr->hbuffer[i]->buffertail == temp)
                        curr->hbuffer[i]->buffertail = prev;
                    temp->next = NULL;
                    curr->bufferSize--;
                    // curr->hbuffer[i]->freq -= temp->freq;
                    sf_delete_buffer(temp);
                }
                prev = curr_buff;
                curr_buff = curr_buff->next;
            }
        }
    }
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
        node = pop(qstack);
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
                    else
                        push(qstack, node->children[child]);
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
        sf_prune_helper(forest[i], forest[i]->data_item, tid);
    }
}

/****************************************************************************/

void sf_print_sfnode(sfnode node)
{
    if(node == NULL)
        return;

    int c = sf_no_sfchildren(node), b = node->bufferSize, idx;

    if(node->parent)
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = %d, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->ftid, node->ltid, node->parent->data_item, c, b);
    else
        printf("data_item = %d, freq = %lf, ftid = %d, ltid = %d, parent = NULL, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->ftid, node->ltid, c, b);

    if(node->hbuffer && node->bufferSize)
    {
        printf("BUFFER TABLE:\n");
        sf_print_buffer_table(node->hbuffer);
    }

    if(node->fptree)
    {
        printf("FPTREE:\n");
        sf_print_fptree(node->fptree->root);
        printf("FP-HEADER TABLE:-\n");
        printf("##############################################################################\n");
        sf_print_header_table(node->fptree->head_table);
        printf("##############################################################################\n");
        printf("******************************************************************************\n");
    }
    
    for(idx = 0; idx < last_index(node->data_item); idx++)
        sf_print_sfnode(node->children[idx]);

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


void sf_print_buffer_table(bufferTable* hbuffer)
{
    int i;
    for(i = 0; i < HSIZE; i++)
    {
        if(hbuffer[i])
        {
            printf(">>>>>>> freq = %lf, tid = %d, bucket = %d, collision = %c<<<<<<<\n",\
                    hbuffer[i]->freq, hbuffer[i]->ltid, i, hbuffer[i]->collision);
            sf_print_buffer(hbuffer[i]->bufferhead);
        }
    }
}


void sf_print_buffer(buffer head)
{
    buffer buff = head;
    while(buff)
    {
        printf("(ftid: %d, ltid: %d); freq = %lf --> ", buff->ftid, buff->ltid, buff->freq);
        sf_print_data_node(buff->itemset);
        buff = buff->next;
    }
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
        // fpnode node = h[idx]->first;
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
    int i;
    for(i = 0; i < d[1]; i++)
    {
        printf("%d ", d[first(d) + i]);
    }
    printf("\n");
}

/****************************************************************************/