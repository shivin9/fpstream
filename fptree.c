/*
1. When deleting a child of a node, fix the child_list as well as the item_list
2. When removing nodes, also fix the header table
3. Header table is now not being recreated from scratch, it is being updated periodically
*/
#include "fptree.h"

int is_equal(data d1, data d2)
{
    return (d1->data_item == d2->data_item);
}


data fp_create_sorted_dummy()
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


fptree fp_create_fptree()
{

    fptree new_tree = calloc(1, sizeof(struct fptree_node));
    if(new_tree == NULL)
    {
        printf("new_tree malloc failed\n");
    }

    fpnode node = calloc(1, sizeof(struct fp_node));
    if(node == NULL)
    {
        printf("node malloc failed\n");
    }

    node->children = NULL;
    node->item_list = NULL;
    node->freq = 0.0;
    node->tid = 0;
    node->data_item = -1;
    node->touched = 0.0;
    node->parent = NULL;
    node->next_similar = NULL;
    node->prev_similar = NULL;
    node->itembuffer = NULL;
    node->bufferSize = 0;

    new_tree->root = node;
    new_tree->head_table = NULL;
    return new_tree;
}

void fp_delete_tree_structure(fpnode current_node)
{

    if(current_node == NULL)
        return;

    fpnode_list current_child_ptr = current_node->children;
    data current_data_ptr = current_node->item_list;
    fpnode this_child;

    while(current_child_ptr != NULL)
    {
        this_child = current_child_ptr->tree_node;

        if(this_child->prev_similar!=NULL)
            (this_child->prev_similar)->next_similar = this_child->next_similar;

        if(this_child->next_similar!=NULL)
            (this_child->next_similar)->prev_similar=(this_child)->prev_similar;

        if(this_child->hnode && this_child->hnode->first == this_child)
            this_child->hnode->first = NULL;

        fp_delete_tree_structure(this_child);
        free(this_child);
        this_child = NULL;

        fpnode_list temp_list = current_child_ptr;
        current_child_ptr = current_child_ptr->next;
        free(temp_list);
        temp_list = NULL;

        data temp_data = current_data_ptr;
        current_data_ptr = current_data_ptr->next;
        free(temp_data);
        temp_data = NULL;
    }
}


void fp_delete_data_node(data d)
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


void fp_delete_header_table(header_table h)
{
    if(h == NULL)    return;
    fp_delete_header_table(h->next);
    free(h);
    h = NULL;
}


void fp_delete_fptree(fptree tree)
{
    if(tree == NULL)    return;
    fp_delete_tree_structure(tree->root);
    free(tree->root);
    tree->root = NULL;
    fp_delete_header_table(tree->head_table);
    free(tree);
    tree = NULL;
}

int fp_size_of_tree(fpnode curr)
{
    if(curr == NULL)
        return 0;
    fpnode_list child = curr->children;
    int size = sizeof(&curr);
    buffer buff = curr->itembuffer;
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

    while(child != NULL)
    {
        size += fp_size_of_tree(child->tree_node);
        child = child->next;
    }
    return size;
}


//////////////////////////////////////////////////////////////////////////


// creates a new node and inserts it into current_node
void fp_create_and_insert_new_child(fpnode current_node, data d, int tid)
{

    fpnode new_node = calloc(1, sizeof(struct fp_node));
    new_node->children = NULL;
    new_node->item_list = NULL;
    new_node->next_similar = NULL;
    new_node->prev_similar = NULL;
    new_node->hnode = NULL;
    new_node->freq = 0.0;
    new_node->tid = tid;
    new_node->touched = 0;
    new_node->data_item = d->data_item;
    new_node->parent = current_node;
    new_node->itembuffer = NULL;
    new_node->bufferSize = 0;

    fpnode_list new_list_node = calloc(1, sizeof(struct fpnode_list_node));
    new_list_node->tree_node = new_node;
    new_list_node->next = NULL;

    data new_data = calloc(1, sizeof(struct data_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;

    //insert of beginning of both linked lists
    new_list_node->next = current_node->children;
    current_node->children = new_list_node;
    new_data->next = current_node->item_list;
    current_node->item_list = new_data;
}

// explicitely inserts the child node in the current node
void fp_insert_new_child(fpnode current_node, fpnode new_child, data d)
{

    new_child->parent = current_node;
    data new_data = calloc(1, sizeof(struct data_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;

    fpnode_list new_list_node = calloc(1, sizeof(struct fpnode_list_node));
    new_list_node->tree_node = new_child;
    new_list_node->next = NULL;

    new_list_node->next = current_node->children;
    current_node->children = new_list_node;
    new_data->next = current_node->item_list;
    current_node->item_list = new_data;
}


////////////////////////////////////////////////////////////////////////////////


int fp_no_children(fpnode current_node)
{
    fpnode_list current_child_ptr = current_node->children;
    int no_children = 0;
    while(current_child_ptr)
    {
        no_children++;
        current_child_ptr = current_child_ptr->next;
    }
    return no_children;
}


int fp_no_dataitem(fpnode current_node)
{
    data current_data_ptr = current_node->item_list;
    int no_dataitem = 0;

    while(current_data_ptr)
    {
        no_dataitem++;
        current_data_ptr = current_data_ptr->next;
    }
    return no_dataitem;
}


int fp_verify_node(fpnode current_node)
{
    printf("\nverification for node %d :-", current_node->data_item);

    fpnode_list current_child_ptr;
    int flag = 1, res = 1;

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
    if(fp_no_children(current_node) != fp_no_dataitem(current_node))
    {
        res = 0;
        printf("not equal number of children and itemlist: %d, %d\n", fp_no_children(current_node), fp_no_dataitem(current_node));
        return res;
    }
    current_child_ptr = current_node->children;
    data current_data_ptr = current_node->item_list;
    while(current_child_ptr)
    {
        int x =  current_child_ptr->tree_node->data_item;
        int y =  current_data_ptr->data_item;
        printf("(%d, %d) ", x, y);
        if(x != y){
            printf("inconsistant children and itemlist: %d, %d\n", fp_no_children(current_node), fp_no_dataitem(current_node));
            return 0;
        }
        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }
    return res;
}


fpnode fp_insert_itemset_helper(fpnode current_node, data d, int tid, int put_in_buffer)
{
    // put_in_buffer tells whether we want to ignore the buffer signal or not
    // d is a single item here and not an itemset
    // if the flag is up then we insert the remaining itemset into the bufferlist of that node and reset the flag
    assert(current_node != NULL);
    extern int leave_as_buffer;

    //updating the frequency of the node according to the formula
    current_node->freq *= pow(DECAY, tid - current_node->tid);
    current_node->freq++;
    current_node->tid = tid;

    if(d == NULL)    return current_node;    //terminate when all items have been inserted

    if(put_in_buffer == 1 && leave_as_buffer)
    {
        printf("leaving in buffer at node %d: ", current_node->data_item);
        fp_print_data_node(d);
        buffer buff = current_node->itembuffer;
        buffer new = (buffer) calloc(1, sizeof(struct buffer_node));

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
        new->next = buff;
        current_node->itembuffer = new;
        current_node->bufferSize++;
        leave_as_buffer = 0;
        return current_node;
    }


    //iterate through children
    //if the next data item has already occurred earlier, go along that child
    //otherwise make a new child
    fpnode_list current_child_ptr = current_node->children, prev = NULL;
    data current_data_ptr = current_node->item_list;

    while(current_child_ptr != NULL)
    {

        fpnode this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d))
        {
            // printf("found match %d\n", d->data_item);
            // fp_update_header_table(htable, d->data_item, tid);
            fpnode after_insert = fp_insert_itemset_helper(this_child, d->next, tid, put_in_buffer);
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }

    // printf("could not find appropriate child for %d:(\n", current_node->data_item);

    //data item has to be inserted as new child
    assert(current_data_ptr == NULL);
    assert(current_child_ptr == NULL);

    fp_create_and_insert_new_child(current_node, d, tid);

    current_child_ptr = current_node->children;
    current_data_ptr = current_node->item_list;
    while(current_child_ptr != NULL)
    {

        fpnode this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d))
        {
            // fp_update_header_table(htable, d->data_item, tid);
            fpnode after_insert = fp_insert_itemset_helper(this_child, d->next, tid, put_in_buffer);
            current_child_ptr->tree_node = after_insert;
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }
}


fptree fp_insert_itemset(fptree tree, data d, int tid, int put_in_buffer)
{
    tree->root = fp_insert_itemset_helper(tree->root, d, tid, put_in_buffer);
    return tree;
}


//////////////////////////////////////////////////////////////////////////////

// this function fixes the next_similar variable also
void fp_create_header_table_helper(fpnode root, header_table* h)
{
    //append this node to the corresponding list for this data item in the header table
    if(root->hnode == NULL)
    {
        data_type this_data = root->data_item;
        header_table curr_header_node = *h;
        while(curr_header_node != NULL)
        {
            if(curr_header_node->data_item == this_data)
            {
                //append to the head of the linked list for this data item
                root->next_similar = curr_header_node->first;
                root->hnode = curr_header_node;
                if(curr_header_node->first)
                    curr_header_node->first->prev_similar = root;

                curr_header_node->first = root;
                /*The first node next to the header table has NO previous node although a next node from the header table points to it*/
                root->prev_similar = NULL;
                // curr_header_node->cnt += root->freq;
            }
            curr_header_node = curr_header_node->next;
        }
    }

    fpnode_list current_child_ptr = root->children;
    while(current_child_ptr != NULL)
    {
        fpnode this_child = current_child_ptr->tree_node;
        fp_create_header_table_helper(this_child, h);
        current_child_ptr = current_child_ptr->next;
    }
}


/*  this function is updating the header table of the fptree*/
void fp_update_header_table(header_table htable, data dat, int tid)
{
    header_table temp, prev;
    data tdat = dat;
    int flag;
    while(tdat)
    {
        temp = htable, prev = NULL, flag = 0;
        while(temp)
        {
            if(temp->data_item == tdat->data_item)
            {
                temp->tid = tid;
                temp->cnt = 0;
                fpnode nxtnode = temp->first;
                while(nxtnode)
                {
                    nxtnode->freq *= pow(DECAY, tid-nxtnode->tid);
                    nxtnode->tid = tid;
                    temp->cnt += nxtnode->freq;
                    nxtnode = nxtnode->next_similar;
                }
                flag = 1;
                break;
            }
            prev = temp;
            temp = temp->next;
        }
        tdat = tdat->next;
    }
}


void fp_create_header_table(fptree tree, int tid)
{
    if(tree == NULL)    return;
    tree->head_table = NULL;
    if(tree->head_table == NULL)
    {
        int cnt;
        header_table new_entry = calloc(1, sizeof(struct header_table_node)), prev = NULL;
        header_table htable = new_entry;

        for(cnt = 0; cnt < DICT_SIZE; cnt++)
        {
            new_entry->data_item = cnt;
            new_entry->first = NULL;
            new_entry->cnt = 0.0;
            new_entry->tid = -1;
            new_entry->next = calloc(1, sizeof(struct header_table_node));
            prev = new_entry;
            new_entry = new_entry->next;
        }
        free(prev->next);
        prev->next = NULL;
        tree->head_table = htable;
    }
    tree->root->parent = NULL;
    fp_create_header_table_helper(tree->root, &(tree->head_table));

    data sorted = fp_create_sorted_dummy();
    fp_update_header_table(tree->head_table, sorted, tid);
    fp_delete_data_node(sorted);
}

//////////////////////////////////////////////////////////////////////////////

// sorts the I-list in DESCENDING order
void fp_sort_header_table(header_table htable, double* table)
{
    if(htable == NULL)
        return;

    header_table temp = htable, nxt;
    fpnode tnode;
    double tcnt;
    data_type tdata_item;

    // insertion sorting the header table
    for(; temp->next != NULL; temp = temp->next)
    {
        for(nxt = temp->next; nxt != NULL; nxt = nxt->next)
        {
            if(temp->cnt < nxt->cnt)
            {
                tdata_item = temp->data_item;
                temp->data_item = nxt->data_item;
                nxt->data_item = tdata_item;

                tdata_item = temp->tid;
                temp->tid = nxt->tid;
                nxt->tid = tdata_item;

                tnode = temp->first;
                temp->first = nxt->first;
                nxt->first = tnode;

                tcnt = temp->cnt;
                temp->cnt = nxt->cnt;
                nxt->cnt = tcnt;

            }
        }
    }

    for(temp = htable; temp != NULL; temp = temp->next)
        // +1 because the index -1 also exists
        table[temp->data_item + 1] = temp->cnt;
}


data fp_reverse_data(data head)
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
void fp_sort_data(data head, double* arr)
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


data fp_array_to_datalist(int* arr, int end)
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


void fp_fix_touched(fpnode node)
{
    if(node == NULL)
        return;

    node->touched = 0;
    fpnode_list child = node->children;
    while(child)
    {
        fp_fix_touched(child->tree_node);
        child = child->next;
    }
}


void fp_convert_helper(fpnode curr, fptree cptree, double* srtd_freqs, int* collected, int tid, int end)
{
    // curr is leaf node
    if((curr->touched == -1 || curr->children == NULL) && curr->freq > 0)
    {
        collected[end] = curr->data_item;
        data head = fp_array_to_datalist(collected, end);
        // need to sort the item using the values in arr
        // fp_print_data_node(head);
        fp_sort_data(head, srtd_freqs);
        cptree = fp_insert_itemset(cptree, head, tid, 0);
        fp_delete_data_node(head);
        curr->freq--;
        // just above the leaf node
        curr = curr->parent;

        while(curr)
        {
            curr->freq--;
            curr = curr->parent;
        }
    }

    else
    {
        collected[end] = curr->data_item;

        fpnode_list child = curr->children;
        fpnode_list first = child;
        int no_children = 0;

        while(child)
        {
            no_children++;
            child = child->next;
        }

        while(no_children)
        {
            if(child == NULL)
                child = first;

            if(child->tree_node->freq > 0)
            {
                fp_convert_helper(child->tree_node, cptree, srtd_freqs, collected, tid, end + 1);
            }

            else if(child->tree_node->freq <= 0)
            {
                no_children--;
                // first = first->next;
                // fp_delete_tree_structure(child->tree_node);
            }
            child = child->next;
        }
        curr->touched = -1;
    }
}


fptree fp_convert_to_CP(fptree tree, int tid)
{
    fpnode curr = tree->root;
    double* srtd_freqs = (double*) malloc(DICT_SIZE*sizeof(double));

    int* collected = (int*) malloc(DICT_SIZE*sizeof(int));
    int end = 0;

    for(end = 0; end < 100; end++)
        srtd_freqs[end] = 0.0;

    if(tree->head_table == NULL)
        fp_create_header_table(tree, tid);

    fp_sort_header_table(tree->head_table, srtd_freqs);
    // int sleepTime = rand()%1000;
    // usleep(sleepTime);
    fptree cptree = fp_create_fptree();
    fp_convert_helper(curr, cptree, srtd_freqs, collected, tid, 0);
    fp_create_header_table(cptree, tid);
    fp_fix_touched(cptree->root);
    fp_sort_header_table(cptree->head_table, srtd_freqs);
    fp_delete_fptree(tree);
    free(srtd_freqs);
    free(collected);
    return cptree;
}


void fp_empty_buffers(fpnode curr)
{
    if(curr == NULL)
        return;
    if(curr->itembuffer)
    {
        assert(curr->bufferSize > 0);
        // printf("\nat node %d", curr->data_item);
        buffer buff = curr->itembuffer, temp;
        while(buff)
        {
            printf("buffer emptied at %d: ", curr->data_item);
            fp_print_data_node(buff->itemset);
            curr = fp_insert_itemset_helper(curr, buff->itemset, buff->tid, 0);
            curr->bufferSize--;
            curr->freq = curr->freq-1;
            fp_delete_data_node(buff->itemset);
            buff->itemset = NULL;
            temp = buff->next;
            free(buff);
            buff = temp;
        }
        curr->itembuffer = NULL;
    }
    fpnode_list child = curr->children;
    while(child)
    {
        // printf("\nemptying child: %d of %d", child->tree_node->data_item, curr->data_item);
        fp_empty_buffers(child->tree_node);
        child = child->next;
    }
}


int fp_ineq7(header_table head, int tid)
{
    double s = N-tid+head->tid;
    double num = pow(DECAY,s-THETA*N)-pow(DECAY,s-2)-(THETA-EPS)*N*(1-DECAY);
    double den = (1-DECAY)*pow(DECAY,s-THETA*N)-(THETA-EPS)*N*pow((1-DECAY),2);
    if((double)tid<=num/den)
        return 1;
    else
        return 0;
}


void fp_update_ancestor(fpnode temp)
{
    fpnode temp1 = temp->parent;
    while(temp1->parent != NULL)
    {
        temp1->freq -= (temp->freq * pow(DECAY, temp1->tid - temp->tid));
        temp1=temp1->parent;
    }
}


void fp_merge1(fpnode parent, fpnode child, int tid)
{

    // child's parent has been detached
    // assert(child->parent->parent == NULL);
    child->parent = parent;
    if(parent->children == NULL)
    {
        assert(parent->item_list == NULL);
        parent->children = calloc(1, sizeof(struct fpnode_list_node));
        parent->children->tree_node = child;
        parent->item_list = calloc(1, sizeof(struct data_node));
        parent->item_list->data_item = child->data_item;
        return;
    }

    fpnode_list childptr = parent->children, prev = NULL;
    fpnode selected_child, tempnode;
    header_table htemp;
    data curr_item = parent->item_list, prev_item = NULL;
    data_type dat = child->data_item;

    while(childptr && childptr->tree_node->data_item != dat)
    {
        prev = childptr;
        prev_item = curr_item;
        assert(prev->tree_node->data_item == prev_item->data_item);
        childptr = childptr->next;
        curr_item = curr_item->next;
    }

    if(childptr == NULL)
    {
        assert(curr_item == NULL);
        prev->next = (fpnode_list) calloc(1, sizeof(struct fpnode_list_node));
        prev->next->tree_node = child;
        prev->next->next = NULL;
        child->parent = parent;

        prev_item->next = (data) calloc(1, sizeof(struct data_node));
        prev_item->next->data_item = child->data_item;
        prev_item->next->next = NULL;
        return;
    }

    assert(childptr != NULL);
    assert(curr_item != NULL);
    assert(childptr->tree_node->data_item == curr_item->data_item);

    selected_child = childptr->tree_node;
    selected_child->freq = selected_child->freq * pow(DECAY, tid - selected_child->tid) + child->freq * pow(DECAY, tid - selected_child->tid);
    selected_child->tid = tid;


    if(child->next_similar != NULL)
        (child->next_similar)->prev_similar = child->prev_similar;

    if(child->prev_similar != NULL)
        (child->prev_similar)->next_similar=child->next_similar;

    else
    {
        header_table htemp = child->hnode;
        htemp->first = child->next_similar;
    }

    fpnode_list child_child = child->children;
    child->children = NULL;
    while(child_child)
    {
        // child_child->tree_node->parent = NULL;
        fp_merge1(selected_child, child_child->tree_node, tid);
        prev = child_child->next;
        free(child_child);
        child_child = prev;
        prev = NULL;
    }
    free(child);
    child = NULL;
    return;
}


void fp_merge(fpnode parent, fpnode child, header_table htable)
{
    // the parent has no children so we assign it a child
    if(parent->children == NULL)
    {
        //assert(fp_verify_node(parent));
        parent->children = (fpnode_list) calloc(1, sizeof(struct fpnode_list_node));
        parent->children->tree_node = child;
        child->parent = parent;
        parent->children->next = NULL;

        parent->item_list = (data) calloc(1, sizeof(struct data_node));
        parent->item_list->data_item = child->data_item;
        parent->item_list->next = NULL;
        //assert(fp_verify_node(parent));
        return;
    }


    data_type nt = child->data_item;
    fpnode_list this_child = parent->children, prev = NULL, child_child;
    fpnode temp = NULL;
    data prntdata = parent->item_list, prevdata = NULL;

    printf("in merge parent %d, children = %d, items = %d: ", parent->data_item, fp_no_children(parent), fp_no_dataitem(parent));
    fp_print_data_node(parent->item_list);

    while(this_child != NULL)
    {
        if(this_child->tree_node->data_item == nt)
        {
            break;
        }
        prev = this_child;
        prevdata = prntdata;
        assert(prevdata->data_item == prev->tree_node->data_item);
        this_child = this_child->next;
        prntdata = prntdata->next;
    }

    // this means that child list not empty but that child does not exist
    if(this_child == NULL)
    {
        assert(prev->next == NULL);
        assert(child != NULL);
        //assert(fp_verify_node(parent));

        int c1 = fp_no_children(parent);
        printf("%d has children but not %d\n", parent->data_item, nt);
        prev->next = (fpnode_list) calloc(1, sizeof(struct fpnode_list_node));
        prev->next->tree_node = child;
        prev->next->next = NULL;
        child->parent = parent;

        prevdata->next = (data) calloc(1, sizeof(struct data_node));
        prevdata->next->data_item = child->data_item;
        prevdata->next->next = NULL;

        printf("%d, added = %d, children = %d, items = %d: ", parent->data_item, nt, fp_no_children(parent), fp_no_dataitem(parent));
        fp_print_data_node(parent->item_list);

        int c2 = fp_no_children(parent);
        //assert(fp_verify_node(parent));
        assert(c2-c1 == 1);
        return;
    }

    printf("\n!!!special merge, child = %d!!!\n", nt);
    temp = this_child->tree_node;
    temp->freq += pow(abs(child->tid - temp->tid), DECAY);
    temp->tid = max(temp->tid, child->tid);

    /* taken cared of by the delete function*/
    if(child->next_similar != NULL)
        (child->next_similar)->prev_similar = child->prev_similar;

    if(child->prev_similar != NULL)
        (child->prev_similar)->next_similar=child->next_similar;

    else
    {
        header_table htemp = htable;
        while(htemp && htemp->data_item != child->data_item)
            htemp = htemp->next;
        htemp->first = child->next_similar;
    }
    //assert(fp_verify_node(parent));

    child_child = child->children;
    while(child_child != NULL)
    {
        //assert(fp_verify_node(temp));
        fp_merge(temp, child_child->tree_node, htable);
        //assert(fp_verify_node(temp));
        prev = child_child;
        child_child = child_child->next;
        // free(prev);
    }
    //assert(fp_verify_node(parent));
    // fp_delete_tree_structure(child);
    // free(child);
}


void fp_prune_infrequent_I_patterns(header_table htable, data_type data_item, int tid)
{
    // printf("\n***pruning data_item %d***\n", data_item);
    header_table htemp = htable;
    while(htemp && htemp->data_item != data_item)
        htemp = htemp->next;

    fpnode fir = htemp->first, temp2, parent;
    htemp->first = NULL;
    fpnode_list temp, ori, temp1;
    data prntdata, tmpdata;
    // fir->parent ? fp_print_tree(fir->parent) : fp_print_tree(fir);

    while(fir != NULL)
    {
        parent = fir->parent;
        // assert(parent->children != NULL);
        // printf("in pruing parent %d, curr_child = %d, children = %d, items = %d: ", parent->data_item, data_item, fp_no_children(parent), fp_no_dataitem(parent));
        // fp_print_data_node(parent->item_list);

        // this code is separating the child from the parent and then we will merge the parent and it's grandchildren
        if(parent->children->tree_node == fir)
        {
            // printf("doing stuff here!\n");
            temp = parent->children;
            tmpdata = parent->item_list;
            parent->children = parent->children->next;
            parent->item_list = parent->item_list->next;
            assert(temp->tree_node == fir);
            free(temp);
            free(tmpdata);
        }
        else
        {
            // printf("doing stuff there!\n");
            temp = parent->children;
            prntdata = parent->item_list;
            ori = NULL;
            while(temp->next != NULL)
            {
                if(temp->next->tree_node == fir)
                {
                    // ori is the child pointer which points to
                    // fir, so we must free it to save space
                    ori = temp->next;
                    tmpdata = prntdata->next;
                    temp->next = temp->next->next;
                    prntdata->next = prntdata->next->next;
                    free(ori);
                    free(tmpdata);
                    break;
                }
                temp = temp->next;
                prntdata = prntdata->next;
            }
        }

        // printf("separated child = %d with %d children: ", fir->data_item, fp_no_children(fir));
        // fp_print_data_node(fir->item_list);

        temp1 = fir->children;

        while(temp1 != NULL)
        {
            // printf("merging %d, %d\n", fir->parent->data_item, temp1->tree_node->data_item);

            // printf("before:- parent %d, children = %d, items = %d: ", parent->data_item, fp_no_children(parent), fp_no_dataitem(parent));

            // fp_print_data_node(parent->item_list);
            // fp_print_tree(parent);
            //assert(fp_verify_node(parent));

            fp_merge1(parent, temp1->tree_node, tid);

            //assert(fp_verify_node(parent));

            // printf("after:- parent %d, children = %d, items = %d: ", parent->data_item, fp_no_children(parent), fp_no_dataitem(parent));

            // fp_print_data_node(parent->item_list);
            // fp_print_tree(parent);

            // this is deleting the children pointers of fir as we will delete it eventually
            temp = temp1;
            temp1 = temp1->next;
            fir->children = temp1;
            temp->next = NULL;
            free(temp);
        }

        temp2 = fir;
        assert(fir != fir->next_similar);
        fir = fir->next_similar;
        // Do not do fp_delete_tree_structure(temp2); it will delete the children of temp2 also which we are merging!

        // if(temp2->prev_similar!=NULL)
        //     (temp2->prev_similar)->next_similar = temp2->next_similar;

        // if(temp2->next_similar!=NULL)
        //     (temp2->next_similar)->prev_similar=(temp2)->prev_similar;

        // if(temp2->hnode && temp2->hnode->first == temp2)
        //     temp2->hnode->first = NULL;

        fp_delete_data_node(temp2->item_list);
        assert(temp2->children == NULL);
        free(temp2);
    }
    // printf("exited:- parent %d, children = %d, items = %d: ", parent->data_item, fp_no_children(parent), fp_no_dataitem(parent));
}


void fp_prune_infrequent_II_patterns(header_table htable, data_type data_item, int tid)
{
    fp_prune_infrequent_I_patterns(htable, data_item, tid);
}


void fp_prune_obsolete_II_patterns(header_table htable, data_type data_item, int tid)
{
    header_table htemp = htable;

    while(htemp && htemp->data_item != data_item)
        htemp = htemp->next;

    fpnode fir = htemp->first, to_free = NULL;
    fpnode_list temp, ori;
    data prntdata;

    while(fir!=NULL)
    {
        if(fir->tid <= tid - N)
        {
            fp_update_ancestor(fir);
            to_free = fir;
            if(to_free->parent->children->tree_node==to_free)
            {
                temp = to_free->parent->children;
                prntdata = to_free->parent->item_list;

                to_free->parent->children=to_free->parent->children->next;
                to_free->parent->item_list = to_free->parent->item_list->next;

                free(temp);
                free(prntdata);
            }

            else
            {
                temp = to_free->parent->children;
                prntdata = to_free->parent->item_list;
                ori = NULL;
                while(temp->next!=NULL)
                {
                    if(temp->next->tree_node == to_free)
                    {
                        ori = temp->next;
                        temp->next=temp->next->next;
                        prntdata->next = prntdata->next->next;
                        free(ori);
                        break;
                    }
                temp = temp->next;
                prntdata = prntdata->next;
                }
            }
            //assert(fp_verify_node(to_free));
            fp_delete_tree_structure(to_free);
            free(to_free);
        }
        fir=fir->next_similar;
    }
}


void fp_prune_obsolete_I_patterns(header_table htable, data_type data_item, int tid)
{
    // printf("***pruning data_item %d***\n", data_item);
    header_table htemp = htable;
    while(htemp && htemp->data_item != data_item)
        htemp = htemp->next;

    fpnode fir = htemp->first, to_free = NULL, parent;
    fpnode_list temp, ori;
    data prntdata;

    while(fir != NULL)
    {
        parent = fir->parent;
        if(fir->freq * pow(DECAY, tid - fir->tid) >= EPS*N)
        {
            fp_update_ancestor(fir);
        }
        to_free = fir;
        // printf("parent %d, children = %d, items = %d: ", parent->data_item, fp_no_children(parent), fp_no_dataitem(parent));
        // fp_print_data_node(parent->item_list);

        fir = fir->next_similar;
        if(to_free->parent->children->tree_node == to_free)
        {
            // printf("doing stuff here\n");
            temp = to_free->parent->children;
            to_free->parent->children=to_free->parent->children->next;
            prntdata = to_free->parent->item_list;
            to_free->parent->item_list = to_free->parent->item_list->next;
            free(temp);
            free(prntdata);
        }
        else
        {
            // printf("doing stuff there\n");
            temp = to_free->parent->children;
            prntdata = to_free->parent->item_list;
            ori = NULL;
            while(temp->next != NULL)
            {
                if(temp->next->tree_node == to_free)
                {
                    ori = temp->next;
                    temp->next = temp->next->next;
                    prntdata->next = prntdata->next->next;
                    free(ori);
                    break;
                }
                temp = temp->next;
                prntdata = prntdata->next;
            }
        }
        // printf("deleting %d, children = %d, items = %d\n", to_free->data_item, fp_no_children(parent), fp_no_dataitem(parent));
        //assert(fp_verify_node(to_free));
        fp_delete_tree_structure(to_free);
        free(to_free);
    }
    htemp->cnt = 0;
    htemp->first = NULL;
    htemp->tid = -1;
}


void fp_prune(fptree ftree, int tid)
{
    header_table htable = ftree->head_table;
    while(htable != NULL)
    {
        if(htable->first != NULL)
        {
            if(htable->tid <= tid-N)
            {
                // printf("pruning obsolete1\n");
                fp_prune_obsolete_I_patterns(htable, htable->data_item, tid);
            }
            else if(htable->tid<tid-N+THETA*N)
            {
                // printf("pruning infrequent1\n");
                fp_prune_infrequent_I_patterns(htable, htable->data_item, tid);
            }
            else if(htable->tid >= tid-N+THETA*N && fp_ineq7(htable, tid))
            {
                // printf("pruning infrequent2\n");
                fp_prune_infrequent_II_patterns(htable, htable->data_item, tid);
            }
            else
            {
                // printf("pruning obsolete2\n");
                fp_prune_obsolete_II_patterns(htable, htable->data_item, tid);
            }
        }
        htable=htable->next;
    }
}


fpnode fp_dfs(fpnode node, data_type highest_priority_data_item)
{

    if(node->touched == 0)    return NULL;

    fpnode new_node = calloc(1, sizeof(struct fp_node));
    // new_node->children = NULL;
    // new_node->item_list = NULL;
    // new_node->touched = 0.0;
    new_node->freq = node->touched;
    new_node->tid = node->tid; // will see this later
    new_node->data_item = node->data_item;
    // new_node->next_similar = NULL;
    // new_node->prev_similar = NULL;
    // new_node->hnode = NULL;
    // new_node->itembuffer = NULL;
    // new_node->bufferSize = 0;

    if(new_node->data_item == highest_priority_data_item)
    {
        node->touched = 0;
        return new_node;
    }

    fpnode_list temp_child_list = node->children;
    data temp_data = node->item_list;
    while(temp_child_list != NULL)
    {
        fpnode this_child = temp_child_list->tree_node;
        if(this_child->touched > 0)
        {
            fpnode new_child = fp_dfs(this_child, highest_priority_data_item);
            if(new_child == NULL)    continue;
            fp_insert_new_child(new_node, new_child, temp_data);
        }

        temp_child_list = temp_child_list->next;
        temp_data = temp_data->next;
    }

    node->touched = 0;
    return new_node;
}


fptree fp_create_conditional_fp_tree(fptree tree, data_type data_item, double minsup, int tid)
{
    header_table curr_head_table_node = tree->head_table;
    fpnode node = NULL;
    while(curr_head_table_node != NULL)
    {
        if(curr_head_table_node->data_item == data_item)
        {
            node = curr_head_table_node->first;
            break;
        }
        curr_head_table_node = curr_head_table_node->next;
    }

    // printf("val of %d = %lf\n", curr_head_table_node->data_item, curr_head_table_node->cnt);

    if(curr_head_table_node == NULL || curr_head_table_node->cnt*pow(DECAY, tid - curr_head_table_node->tid) < minsup){
        return NULL;
    }

    if(node == NULL){
        return NULL;
    }

    // printf("success %d %d %lf\n", data_item, node->data_item, node->freq);

    //node is the link to successive fpnodes having data type 'data_item'
    //iterate through it and for each node in it, start from that node
    //and touch all nodes till the root
    //touched nodes are a means of identifying which nodes should be in coditional FP-tree
    double add;
    while(node != NULL)
    {
        fpnode temp = node;
        add = temp->freq * pow(DECAY, tid - temp->tid);
        // temp->touched = 0;
        while(temp != NULL)
        {
            temp->touched += add;
            // printf("touched = %d\n", temp->touched);
            temp = temp->parent;
        }
        node = node->next_similar;
    }

    // printf("after touching:\n");
    // fp_print_tree(tree->root);

    //now run a DFS from the root of the given FP_tree, for all touched nodes,
    //create a copy for the conditional FP-tree

    fpnode cond_fptree = fp_dfs(tree->root, data_item);
    if(cond_fptree == NULL){
        // printf("condtree is null!!!\n");
        return NULL;
    }

    // printf("cond_fptree\n");
    // fp_print_tree(cond_fptree);
    // printf("\n");

    fptree cond_tree = calloc(1, sizeof(struct fptree_node));
    cond_tree->root = cond_fptree;
    cond_tree->head_table = NULL;
    fp_create_header_table(cond_tree, tid);
    // fp_print_header_table(cond_tree->head_table);
    assert(cond_tree != NULL);
    return cond_tree;
}


void fp_mine_frequent_itemsets(fptree tree, data sorted, data till_now, int tid, int pattern)
{
    if(tree == NULL)    return;

    // if(till_now == NULL)
    //     printf("\nentered new mine with till_now = NULL and sorted = %d\n", sorted->data_item);
    // else if(sorted == NULL)
    //     printf("\nentered new mine sorted = NULL\n");
    // else
    //     printf("\nentered new mine sorted = %d\n", sorted->data_item);

    header_table curr_header_node = tree->head_table;

    if(till_now != NULL)
    {
        // printf("till_now = ");
        data temp = till_now;
        // fp_print_data_node(till_now);

        //current itemset is not empty
        data last_item = till_now;

        while(last_item->next != NULL)
            last_item = last_item->next;

        while(curr_header_node != NULL)
        {
            if(curr_header_node->data_item == last_item->data_item)    break;
            curr_header_node = curr_header_node->next;
        }

        // printf("head->cnt = %lf\n", curr_header_node->cnt);
        if((pattern == 0 && curr_header_node->cnt >= MINSUP_SEMIFREQ)
                || (pattern == 1 && curr_header_node->cnt >= MINSUP_FREQ)
                || (pattern == 2 && curr_header_node->cnt >= SUP))
        {
            //frequent itemset
            // printf("***writing to file***\n");
            FILE *fp;
            if(pattern%2 == 0)
                fp = fopen("intermediate", "a");
            else
                fp = fopen("output", "a");

            int t = 0, arr[DICT_SIZE];

            data temp = till_now;
            // temp = temp->next;
            while(temp != NULL)
            {
                // printf("%d ", temp->data_item);
                arr[t++] = temp->data_item;
                temp = temp->next;
            }
            // size of frequent itemset
            fprintf(fp, "%d", t);
            // printf("%d", t);
            t--;
            while(t >= 0)
            {
                fprintf(fp, " %d", arr[t]);
                // printf(" %d", arr[t]);
                t--;
            }
            if(pattern%2 == 0){
                fprintf(fp, " %lf", curr_header_node->cnt);
                // printf(" %lf", curr_header_node->cnt);
            }
            // printf("\n");

            fprintf(fp, "\n");
            fclose(fp);
            // printf("printed to intermediate\n");
        }
    }

    if(sorted == NULL)    return;

    //now check for supersets of this itemset by considering every next data item
    //in the sorted list
    data curr_data = sorted;
    while(curr_data != NULL)
    {
        // printf("going to get condtree\n");
        fptree cond_tree;
        if(pattern == 2)
            cond_tree = fp_create_conditional_fp_tree(tree, curr_data->data_item, SUP, tid);

        else
            cond_tree = fp_create_conditional_fp_tree(tree, curr_data->data_item,
                (pattern == 1) ? MINSUP_FREQ : MINSUP_SEMIFREQ, tid);

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
        else{
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
        //     fp_print_tree(cond_tree->root);

        // else
        //     printf("NULL tree\n");

        // if(curr_data->next != NULL)
        //     printf("going to mine %d\n", curr_data->next->data_item);
        // else
        //     printf("going to mine NULL\n");

        fp_mine_frequent_itemsets(cond_tree, curr_data->next, till_now, tid, pattern);
        fp_delete_fptree(cond_tree);

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


//////////////////////////////////////////////////////////////////////////////


void fp_print_node(fpnode node)
{
    if(node == NULL)    return;
    int c = fp_no_children(node), b = node->bufferSize;

    if(node->data_item != -1)
        printf("data_item = %d, freq = %lf, tid = %d, parent = %d, touched = %lf, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->tid, node->parent->data_item, node->touched, c, b);
    else
        printf("data_item = %d, freq = %lf, tid = %d, parent = NULL, touched = %lf, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->tid, node->touched, c, b);

    printf("BUFFER:\n");
    buffer buff = node->itembuffer;
    while(buff)
    {
        fp_print_data_node(buff->itemset);
        buff = buff->next;
    }
}


void fp_print_tree(fpnode node)
{
    fp_print_node(node);
    fpnode_list curr_child_list = node->children;
    fpnode this_child;
    while(curr_child_list != NULL)
    {
        this_child = curr_child_list->tree_node;
        // printf("going to child %d %d\n", this_child->data_item, curr_data->data_item);
        fp_print_tree(this_child);
        curr_child_list = curr_child_list->next;
    }
}


void fp_print_header_table(header_table h)
{
    int z = 0;
    // z is the size of the table
    while(h != NULL)
    {
        printf("%d %d %lf\n", h->data_item, h->tid, h->cnt);
        fpnode node = h->first;
        while(node != NULL && node->next_similar != node)
        {
            // fp_print_node(node);
            node = node->next_similar;
            // z++;
        }
        // printf("\n");
        h = h->next;
    }
}

void fp_print_data_node(data d)
{
    while(d != NULL)
    {
        printf("%d ", d->data_item);
        d = d->next;
    }
    printf("\n");
}

//////////////////////////////////////////////////////////////////////////////
