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
	sforest sfrst = calloc(DICT_SIZE, sizeof(struct sftree));
	for(i = 0; i < DICT_SIZEl i++)
	{
		sfrst[i] = sf_create_sftree();
	}
	return sfrst;
}

sftree sf_create_sftree()
{

    sftree new_tree = calloc(1, sizeof(struct sftree_node));
    if(new_tree == NULL)
    {
        printf("new_tree malloc failed\n");
    }

    sfnode node = calloc(1, sizeof(struct sf_node));
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

void sf_delete_tree_structure(sfnode current_node)
{

    if(current_node == NULL)
        return;

    sfnode_list current_child_ptr = current_node->children;
    data current_data_ptr = current_node->item_list;
    sfnode this_child;

    while(current_child_ptr != NULL)
    {
        this_child = current_child_ptr->tree_node;

        if(this_child->prev_similar!=NULL)
            (this_child->prev_similar)->next_similar = this_child->next_similar;

        if(this_child->next_similar!=NULL)
            (this_child->next_similar)->prev_similar=(this_child)->prev_similar;

        if(this_child->hnode && this_child->hnode->first == this_child)
            this_child->hnode->first = NULL;

        sf_delete_tree_structure(this_child);
        free(this_child);
        this_child = NULL;

        sfnode_list temp_list = current_child_ptr;
        current_child_ptr = current_child_ptr->next;
        free(temp_list);
        temp_list = NULL;

        data temp_data = current_data_ptr;
        current_data_ptr = current_data_ptr->next;
        free(temp_data);
        temp_data = NULL;
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


void sf_delete_header_table(header_table h)
{
    if(h == NULL)    return;
    sf_delete_header_table(h->next);
    free(h);
    h = NULL;
}


void sf_delete_sftree(sftree tree)
{
    if(tree == NULL)    return;
    sf_delete_tree_structure(tree->root);
    free(tree->root);
    tree->root = NULL;
    sf_delete_header_table(tree->head_table);
    free(tree);
    tree = NULL;
}

int sf_size_of_tree(sfnode curr)
{
    if(curr == NULL)
        return 0;
    sfnode_list child = curr->children;
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
        size += sf_size_of_tree(child->tree_node);
        child = child->next;
    }
    return size;
}

data sf_copy_data_node(data ori)
{
    data new = NULL, temp = ori, head = new;
    while(temp)
    {
        new = calloc(1, sizeof(struct data_node));
        new->data_item = temp->data_item;
        temp = temp->next;
        new = new->next;
    }
    return head;
}

//////////////////////////////////////////////////////////////////////////


// creates a new node and inserts it into current_node
void sf_create_and_insert_new_child(sfnode current_node, data d, int tid)
{

    sfnode new_node = calloc(1, sizeof(struct sf_node));
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

    sfnode_list new_list_node = calloc(1, sizeof(struct sfnode_list_node));
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
void sf_insert_new_child(sfnode current_node, sfnode new_child, data d)
{

    new_child->parent = current_node;
    data new_data = calloc(1, sizeof(struct data_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;

    sfnode_list new_list_node = calloc(1, sizeof(struct sfnode_list_node));
    new_list_node->tree_node = new_child;
    new_list_node->next = NULL;

    new_list_node->next = current_node->children;
    current_node->children = new_list_node;
    new_data->next = current_node->item_list;
    current_node->item_list = new_data;
}


////////////////////////////////////////////////////////////////////////////////


int sf_no_children(sfnode current_node)
{
    sfnode_list current_child_ptr = current_node->children;
    int no_children = 0;
    while(current_child_ptr)
    {
        no_children++;
        current_child_ptr = current_child_ptr->next;
    }
    return no_children;
}


int sf_no_dataitem(sfnode current_node)
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


int sf_verify_node(sfnode current_node)
{
    printf("\nverification for node %d :-", current_node->data_item);

    sfnode_list current_child_ptr;
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
    if(sf_no_children(current_node) != sf_no_dataitem(current_node))
    {
        res = 0;
        printf("not equal number of children and itemlist: %d, %d\n", sf_no_children(current_node), sf_no_dataitem(current_node));
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
            printf("inconsistant children and itemlist: %d, %d\n", sf_no_children(current_node), sf_no_dataitem(current_node));
            return 0;
        }
        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }
    return res;
}


sfnode sf_insert_itemset_helper(sfnode current_node, header_table htable, data d, int tid, int put_in_buffer)
{
    // put_in_buffer tells whether we want to ignore the buffer signal or not
    // d is a single item here and not an itemset
    // if the flag is up then we insert the remaining itemset into the bufferlist of that node and reset the flag
    assert(current_node != NULL);
    // assert(htable != NULL);
    extern int leave_as_buffer;

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

    if(put_in_buffer == 1 && leave_as_buffer)
    {
        printf("leaving in buffer at node %d: ", current_node->data_item);
        sf_print_data_node(d);
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
    sfnode_list current_child_ptr = current_node->children, prev = NULL;
    data current_data_ptr = current_node->item_list;

    while(current_child_ptr != NULL)
    {
        sfnode this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d))
        {
            // printf("found match %d\n", d->data_item);
            // sf_update_header_table(htable, d->data_item, tid);
            sfnode after_insert = sf_insert_itemset_helper(this_child, htable, d->next, tid, put_in_buffer);
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }

    // printf("could not find appropriate child for %d:(\n", current_node->data_item);

    //data item has to be inserted as new child
    assert(current_data_ptr == NULL);
    assert(current_child_ptr == NULL);

    sf_create_and_insert_new_child(current_node, d, tid);

    current_child_ptr = current_node->children;
    current_data_ptr = current_node->item_list;

    while(current_child_ptr != NULL)
    {
        sfnode this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d))
        {
            sfnode after_insert = sf_insert_itemset_helper(this_child, htable, d->next, tid, put_in_buffer);
            current_child_ptr->tree_node = after_insert;
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }
}


sftree sf_insert_itemset(sforest forest, data d, int tid, int put_in_buffer)
{
	sftree tree = forest[d->data_item]; /* select the tree to insert*/
    tree->root = sf_insert_itemset_helper(tree->root, tree->head_table, d, tid, put_in_buffer);
    return tree;
}


//////////////////////////////////////////////////////////////////////////////

// this function fixes the next_similar variable also
void sf_create_header_table_helper(sfnode root, header_table h)
{
    //append this node to the corresponding list for this data item in the header table
    if(root->hnode == NULL)
    {
        data_type this_data = root->data_item;
        header_table curr_header_node = h;
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
                break;
                // curr_header_node->cnt += root->freq;
            }
            curr_header_node = curr_header_node->next;
        }
    }

    sfnode_list current_child_ptr = root->children;
    while(current_child_ptr != NULL)
    {
        sfnode this_child = current_child_ptr->tree_node;
        sf_create_header_table_helper(this_child, h);
        current_child_ptr = current_child_ptr->next;
    }
}


/*  this function is updating the header table of the sftree*/
void sf_update_header_table(header_table htable, data dat, int tid)
{
    header_table temp, prev;
    data tdat = dat;
    while(tdat)
    {
        temp = htable;
        while(temp && temp->data_item != tdat->data_item)
            temp = temp->next;

        temp->cnt = 0;
        sfnode nxtnode = temp->first;
        while(nxtnode)
        {
            temp->tid = max(temp->tid, nxtnode->tid);
            temp->cnt += nxtnode->freq * pow(DECAY, tid - nxtnode->tid);
            nxtnode = nxtnode->next_similar;
        }
        tdat = tdat->next;
    }
}


void sf_create_header_table(sftree tree, int tid)
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
    sf_create_header_table_helper(tree->root, tree->head_table);

    data sorted = sf_create_sorted_dummy();
    sf_update_header_table(tree->head_table, sorted, tid);
    sf_delete_data_node(sorted);
}

//////////////////////////////////////////////////////////////////////////////

// sorts the I-list in DESCENDING order
void sf_sort_header_table(header_table htable, double* table)
{
    if(htable == NULL)
        return;

    header_table temp = htable, nxt;
    sfnode tnode;
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
    sfnode_list child = node->children;
    while(child)
    {
        sf_fix_touched(child->tree_node);
        child = child->next;
    }
}


void sf_empty_buffers(sfnode curr, header_table htable, int tid)
{
}

//////////////////////////////////////////////////////////////////////////////


void sf_print_node(sfnode node)
{
    if(node == NULL)    return;
    int c = sf_no_children(node), b = node->bufferSize;

    if(node->data_item != -1)
        printf("data_item = %d, freq = %lf, tid = %d, parent = %d, touched = %lf, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->tid, node->parent->data_item, node->touched, c, b);
    else
        printf("data_item = %d, freq = %lf, tid = %d, parent = NULL, touched = %lf, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->tid, node->touched, c, b);

    printf("BUFFER:\n");
    buffer buff = node->itembuffer;
    while(buff)
    {
        sf_print_data_node(buff->itemset);
        buff = buff->next;
    }
}


void sf_print_tree(sfnode node)
{
    sf_print_node(node);
    sfnode_list curr_child_list = node->children;
    sfnode this_child;
    while(curr_child_list != NULL)
    {
        this_child = curr_child_list->tree_node;
        // printf("going to child %d %d\n", this_child->data_item, curr_data->data_item);
        sf_print_tree(this_child);
        curr_child_list = curr_child_list->next;
    }
}


void sf_print_header_table(header_table h)
{
    int z = 0;
    // z is the size of the table
    while(h != NULL)
    {
        printf("%d %d %lf\n", h->data_item, h->tid, h->cnt);
        sfnode node = h->first;
        while(node != NULL && node->next_similar != node)
        {
            // sf_print_node(node);
            node = node->next_similar;
            // z++;
        }
        // printf("\n");
        h = h->next;
    }
}

void sf_print_data_node(data d)
{
    while(d != NULL)
    {
        printf("%d ", d->data_item);
        d = d->next;
    }
    printf("\n");
}

//////////////////////////////////////////////////////////////////////////////
