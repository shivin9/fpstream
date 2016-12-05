#include "fptree.h"

int min(int a, int b)
{
    if(a < b)    return a;
    return b;
}

int max(int a, int b)
{
    if(a > b)    return a;
    return b;
}

int is_equal(data d1, data d2)
{
    return (d1->data_item == d2->data_item);
}


fptree fp_create_fptree()
{

    fptree new_tree = malloc(sizeof(struct fptree_node));
    if(new_tree == NULL)
    {
        printf("new_tree malloc failed\n");
    }

    fpnode node = malloc(sizeof(struct fp_node));
    if(node == NULL)
    {
        printf("node malloc failed\n");
    }

    node->children = NULL;
    node->item_list = NULL;
    node->freq = 0;
    node->data_item = -1;
    node->touched = 0;
    node->parent = NULL;
    node->next_similar = NULL;
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

    while(current_child_ptr != NULL)
    {

        fpnode this_child = current_child_ptr->tree_node;
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

void fp_delete_data_node(data d){
    data temp;
    while(d){
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

    while(buff){
        size += sizeof(buff);
        temp = buff->itemset;
        while(temp){
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
void fp_create_and_insert_new_child(fpnode current_node, data d)
{

    fpnode new_node = malloc(sizeof(struct fp_node));
    new_node->children = NULL;
    new_node->item_list = NULL;
    new_node->next_similar = NULL;
    new_node->freq = new_node->touched = 0;
    new_node->data_item = d->data_item;
    new_node->parent = current_node;
    new_node->itembuffer = NULL;
    new_node->bufferSize = 0;

    fpnode_list new_list_node = malloc(sizeof(struct fpnode_list_node));
    new_list_node->tree_node = new_node;
    new_list_node->next = NULL;

    data new_data = malloc(sizeof(struct data_node));
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
    data new_data = malloc(sizeof(struct data_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;

    fpnode_list new_list_node = malloc(sizeof(struct fpnode_list_node));
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

fpnode fp_insert_itemset_helper(fpnode current_node, data d, int put_in_buffer)
{
    // put_in_buffer tells whether we want to ignore the buffer signal or not
    // d is a single item here and not an itemset
    // if the flag is up then we insert the remaining itemset into the bufferlist of that node and reset the flag
    extern int leave_as_buffer;
    assert(current_node != NULL);
    //increment frequency of this node of fp-tree
    current_node->freq++;
    if(d == NULL)    return current_node;    //terminate when all items have been inserted

    if(put_in_buffer == 1 && leave_as_buffer)
    {
        printf("leaving in buffer at node %d: ", current_node->data_item);
        fp_print_data_node(d);
        buffer buff = current_node->itembuffer;
        buffer new = (buffer) malloc(sizeof(struct buffer_node));

        data temp;
        new->itemset = (data) malloc(sizeof(struct data_node));
        new->itemset->data_item = d->data_item;
        temp = new->itemset;
        d = d->next;
        while(d){
            temp->next = (data) malloc(sizeof(struct data_node));
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
            fpnode after_insert = fp_insert_itemset_helper(this_child, d->next, put_in_buffer);

            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }

    // printf("could not find appropriate child for %d:(\n", current_node->data_item);

    //data item has to be inserted as new child
    assert(current_data_ptr == NULL);
    assert(current_child_ptr == NULL);

    fp_create_and_insert_new_child(current_node, d);

    current_child_ptr = current_node->children;
    current_data_ptr = current_node->item_list;
    while(current_child_ptr != NULL)
    {

        fpnode this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d))
        {
            fpnode after_insert = fp_insert_itemset_helper(this_child, d->next, put_in_buffer);
            current_child_ptr->tree_node = after_insert;
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }
}


fptree fp_insert_itemset(fptree tree, data d, int put_in_buffer)
{
    // extern int leave_as_buffer = 0;
    int len = 0;
    data temp = d;
    while(temp){
        len++;
        temp = temp->next;
    }
    tree->root = fp_insert_itemset_helper(tree->root, d, put_in_buffer);
    return tree;
}


//////////////////////////////////////////////////////////////////////////////

// this function fixes the next_similar variable also
void fp_create_header_table_helper(fpnode root, header_table* h)
{
    //append this node to the corresponding list for this data item in the header table
    data_type this_data = root->data_item;

    header_table curr_header_node = *h;
    int found = 0;
    while(curr_header_node != NULL)
    {
        if(curr_header_node->data_item == this_data)
        {

            //append to the head of the linked list for this data item
            root->next_similar = curr_header_node->first;
            curr_header_node->first = root;
            curr_header_node->cnt += root->freq;

            found = 1;
            break;
        }

        curr_header_node = curr_header_node->next;
    }

    if(found == 0)
    {
        //create new entry in header table
        header_table new_entry = malloc(sizeof(struct header_table_node));
        new_entry->data_item = this_data;
        new_entry->first = root;
        new_entry->cnt = root->freq;
        new_entry->next = *h;
        *h = new_entry;
    }

    fpnode_list current_child_ptr = root->children;

    while(current_child_ptr != NULL)
    {
        fpnode this_child = current_child_ptr->tree_node;
        fp_create_header_table_helper(this_child, h);
        current_child_ptr = current_child_ptr->next;
    }
}


void fp_create_header_table(fptree tree)
{
    if(tree == NULL)    return;
    tree->head_table = NULL;
    tree->root->parent = NULL;
    fp_create_header_table_helper(tree->root, &(tree->head_table));
}

//////////////////////////////////////////////////////////////////////////////


fpnode fp_dfs(fpnode node, data_type highest_priority_data_item)
{

    if(node->touched == 0)    return NULL;

    fpnode new_node = malloc(sizeof(struct fp_node));
    new_node->children = NULL;
    new_node->item_list = NULL;
    new_node->touched = 0;
    new_node->freq = node->touched;
    new_node->data_item = node->data_item;
    new_node->next_similar = NULL;
    new_node->itembuffer = NULL;
    new_node->bufferSize = 0;

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

// sorts the I-list in descending order
void fp_sort_header_table(header_table htable, int* table)
{
    if(htable == NULL)
        return;

    header_table temp = htable, nxt;
    fpnode tnode;
    int tcnt;
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

data fp_reverse_data(data head){
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
void fp_sort_data(data head, int* arr)
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
    data head = (data) malloc(sizeof(struct data_node));
    data temp = head;
    head->data_item = arr[1];
    head->next = NULL;
    for(i = 2; i <= end; i++)
    {
        temp->next = (data) malloc(sizeof(struct data_node));
        temp = temp->next;
        temp->data_item = arr[i];
        temp->next = NULL;
    }
    return head;
}

void fp_fix_touched(fpnode node){
    if(node == NULL)
        return;

    node->touched = 0;
    fpnode_list child = node->children;
    while(child){
        fp_fix_touched(child->tree_node);
        child = child->next;
    }
}

void fp_convert_helper(fpnode curr, fptree cptree, int* arr, int* collected, int end)
{
    // curr is leaf node
    if((curr->touched == -1 || curr->children == NULL) && curr->freq > 0)
    {
        collected[end] = curr->data_item;
        data head = fp_array_to_datalist(collected, end);
        // need to sort the item using the values in arr
        // fp_print_data_node(head);
        fp_sort_data(head, arr);
        cptree = fp_insert_itemset(cptree, head, 0);
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

    else{
        collected[end] = curr->data_item;

        fpnode_list child = curr->children;
        fpnode_list first = child;
        int no_children = 0, counter = curr->freq;

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
                fp_convert_helper(child->tree_node, cptree, arr, collected, end + 1);
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

fptree fp_convert_to_CP(fptree tree)
{
    fpnode curr = tree->root;
    int* arr = (int*) malloc(DICT_SIZE*sizeof(int));
    int* collected = (int*) malloc(DICT_SIZE*sizeof(int));
    int end = 0;

    for(end = 0; end < 100; end++)
        arr[end] = 0;

    if(tree->head_table == NULL)
        fp_create_header_table(tree);

    fp_sort_header_table(tree->head_table, arr);
    // int sleepTime = rand()%1000;
    // usleep(sleepTime);
    fptree cptree = fp_create_fptree();
    fp_convert_helper(curr, cptree, arr, collected, 0);
    fp_create_header_table(cptree);
    fp_fix_touched(cptree->root);
    fp_sort_header_table(cptree->head_table, arr);
    fp_delete_fptree(tree);
    free(arr);
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
            curr = fp_insert_itemset_helper(curr, buff->itemset, 0);
            curr->bufferSize--;
            curr->freq--;
            fp_delete_data_node(buff->itemset);
            buff->itemset = NULL;
            temp = buff->next;
            free(buff);
            buff = NULL;
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


fptree fp_create_conditional_fp_tree(fptree tree, data_type data_item, int minsup)
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

    if(curr_head_table_node == NULL || curr_head_table_node->cnt < minsup)
        return NULL;
    if(node == NULL)    return NULL;

    // printf("%d\n", curr_head_table_node->cnt);
    // printf("success %d %d %d\n", data_item, node->data_item, node->freq);

    //node is the link to successive fpnodes having data type 'data_item'
    //iterate through it and for each node in it, start from that node
    //and touch all nodes till the root
    //touched nodes are a means of identifying which nodes should be in coditional FP-tree

    while(node != NULL)
    {
        fpnode temp = node;
        int add = temp->freq;
        while(temp != NULL)
        {
            temp->touched += add;
            temp = temp->parent;
        }
        node = node->next_similar;
    }

    // printf("after touching:\n");
    // print_tree(tree->root);
    // printf("MINSUP = %d\n", MINSUP);

    //now run a DFS from the root of the given FP_tree, for all touched nodes,
    //create a copy for the conditional FP-tree

    fpnode cond_fptree = fp_dfs(tree->root, data_item);
    if(cond_fptree == NULL)    return NULL;

    // printf("cond_fptree\n");
    // print_tree(cond_fptree);
    // printf("\n");

    fptree cond_tree = malloc(sizeof(struct fptree_node));
    cond_tree->root = cond_fptree;
    cond_tree->head_table = NULL;
    fp_create_header_table(cond_tree);
    return cond_tree;
}


void fp_mine_frequent_itemsets(fptree tree, data sorted, data till_now, int pattern)
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

        assert(curr_header_node != NULL);
        if((pattern == 0 && curr_header_node->cnt >= MINSUP_SEMIFREQ)
                || (pattern == 1 && curr_header_node->cnt >= MINSUP_FREQ))
        {

            //frequent itemset
            FILE *fp;
            if(pattern == 0)
                fp = fopen("intermediate", "a");
            else
                fp = fopen("output", "a");

            int t=0, arr[DICT_SIZE];

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
            if(pattern == 0){
                fprintf(fp, " %d", curr_header_node->cnt);
                // printf(" %d", curr_header_node->cnt);
            }
            // printf("\n");

            fprintf(fp, "\n");
            fclose(fp);
            // printf("printed to intermediate\n");
        }
    }
    else{

    }

    if(sorted == NULL)    return;

    //now check for supersets of this itemset by considering every next data item
    //in the sorted list
    data curr_data = sorted;
    while(curr_data != NULL)
    {

        fptree cond_tree = fp_create_conditional_fp_tree(tree, curr_data->data_item,
                (pattern == 1) ? MINSUP_FREQ : MINSUP_SEMIFREQ);
        if(cond_tree == NULL)
        {
            // printf("skipped %d\n", curr_data->data_item);
            curr_data = curr_data->next;
            continue;
        }

        //append to front of current itemset
        data new_data = malloc(sizeof(struct data_node));
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

        fp_mine_frequent_itemsets(cond_tree, curr_data->next, till_now, pattern);
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
        printf("data_item = %d, freq = %d, parent = %d, touched = %d, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->parent->data_item, node->touched, c, b);
    else
        printf("data_item = %d, freq = %d, parent = NULL, touched = %d, children = %d, buffer_size = %d \n", node->data_item, node->freq, node->touched, c, b);

    printf("BUFFER:\n");
    buffer buff = node->itembuffer;
    while(buff){
        fp_print_data_node(buff->itemset);
        buff = buff->next;
    }
}


void fp_print_tree(fpnode node)
{
    fp_print_node(node);
    fpnode_list curr_child_list = node->children;
    data curr_data = node->item_list;
    while(curr_data != NULL)
    {
        fpnode this_child = curr_child_list->tree_node;
        // printf("going to child %d %d\n", this_child->data_item, curr_data->data_item);
        fp_print_tree(this_child);
        curr_child_list = curr_child_list->next;
        curr_data = curr_data->next;
    }
}


void fp_print_header_table(header_table h)
{
    int z = 0;
    // z is the size of the table
    while(h != NULL)
    {
        printf("%d %d\n", h->data_item, h->cnt);
        fpnode node = h->first;
        while(node != NULL)
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

void fp_print_dots(int num){
    while(num--)
        printf("|");
}

void fp_print_tree_data(fpnode_list qhead, fpnode_list qtail){
    fpnode_list child, temp;
    fpnode curr = qhead->tree_node;
    int size = 1;
    while(size > 0){
        child = curr->children;
        printf("%d, %d->", curr->data_item, curr->freq);
        while(child){
            qtail->next = (fpnode_list) malloc(sizeof(struct fpnode_list_node));
            qtail = qtail->next;
            printf("%d, %d->", qtail->tree_node->data_item, qtail->tree_node->freq);
            size++;
        }
        curr = qhead->tree_node;
        if(qhead->next){
            qhead = qhead->next;
            size--;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
