#include "pattern_tree.h"

pdata create_sorted_dummy()
{
    pdata d = NULL;
    int next = 0;
    while(next < DICT_SIZE){
        pdata new_d = malloc(sizeof(struct pdata_node));
        new_d->data_item = next++;
        new_d->next = d;
        d = new_d;
    }
    return d;
}


int is_equal(pdata d1, pdata d2)
{
    if(d1->data_item == d2->data_item)
        return 1;
    return 0;
}

patterntree create_pattern_tree()
{
    patterntree new_tree = malloc(sizeof(struct pattern_tree));
    pattern_node new_node = create_new_pattern_node(-1);
    new_tree->root = new_node;
    return new_tree;
}


pattern_node create_new_pattern_node(data_type d)
{
    pattern_node new_node = malloc(sizeof(struct pattern_node_));
    new_node->children = NULL;
    new_node->item_list = NULL;
    new_node->data_item = d;
    new_node->table = NULL;
    return new_node;
}

void delete_pattern_node(pattern_node pnode)
{
    pattern_node_list child = pnode->children, prev;
    while(child)
    {
        prev = child;
        child = child->next;
        delete_pattern_node(prev->tree_node);
        free(prev);
    }
    delete_tail_freq_table(pnode->table);
    fp_delete_data_node(pnode->item_list);
    // free(pnode->data_item);
    free(pnode);
}

void delete_pattern_tree(patterntree ptree){
    delete_pattern_node(ptree->root);
    free(ptree);
}


//////////////////////////////////////////////////////////////////////////////

void create_and_insert_new_child(pattern_node current_node, pdata d)
{

    pattern_node new_node = create_new_pattern_node(d->data_item);

    pattern_node_list new_list_node = malloc(sizeof(struct pattern_node_list_node));
    new_list_node->tree_node = new_node;
    new_list_node->next = NULL;

    pdata new_data = malloc(sizeof(struct pdata_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;

    //insert of beginning of both linked lists
    new_list_node->next = current_node->children;
    current_node->children = new_list_node;
    new_data->next = current_node->item_list;
    current_node->item_list = new_data;
}


////////////////////////////////////////////////////////////////////////////////

pattern_node insert_itemset_helper(pattern_node current_node, pdata d, int batch_num, float add)
{

    assert(current_node != NULL);

    // if(d == NULL)
    // {
    //     printf("inserted transaction\n\n");
    // }
    // else{
    //     printf("about to insert %d with freq %d, currently at node: ", d->data_item, add);
    //     print_node(current_node);
    // }

    update_tilted_tw_table(current_node, batch_num, add);


    // if(d == NULL)
    // {
    //     printf("inserted transaction\n\n");
    // }
    // else{
    //     printf("about to insert %d with freq %d, currently at node: ", d->data_item, add);
    //     print_node(current_node);
    // }

    if(d == NULL)    return current_node;    //terminate when all items have been inserted

    //iterate through children
    //if the next pdata item has already occurred earlier, go along that child
    //otherwise make a new child

    pattern_node_list current_child_ptr = current_node->children;
    pdata current_data_ptr = current_node->item_list;

    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        pdata this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d) == 1)
        {

            pattern_node after_insert = insert_itemset_helper(this_child, d->next, batch_num, add);
            current_child_ptr->tree_node = after_insert;
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }


    // printf("could not find appropriate child :(\n");

    //pdata item has to be inserted as new child
    assert(current_data_ptr == NULL);
    assert(current_child_ptr == NULL);

    create_and_insert_new_child(current_node, d);

    current_child_ptr = current_node->children;
    current_data_ptr = current_node->item_list;
    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        pdata this_data_item = current_data_ptr;

        if(is_equal(this_data_item, d))
        {
            pattern_node after_insert = insert_itemset_helper(this_child, d->next, batch_num, add);
            current_child_ptr->tree_node = after_insert;
            return current_node;
        }

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }
    assert(0);
}

pdata data_2_pdata(data d)
{
    int i = d[1] - 1;
    pdata pd = NULL;
    while(i >= 2) /* data d is in 'smart' form */
    {
        pdata new_d = malloc(sizeof(struct pdata_node));
        new_d->data_item = d[i];
        printf("%d, ", d[i]);
        new_d->next = pd;
        pd = new_d;
        i--;
    }
    return pd;
}

patterntree insert_itemset(patterntree tree, pdata d, int batch_num, float add)
{
    tree->root = insert_itemset_helper(tree->root, d, batch_num, add);
    return tree;
}


//////////////////////////////////////////////////////////////////////////////


tilted_tw_table create_new_tilted_tw_table(int starting_batch, int ending_batch, float add)
{
    tilted_tw_table new_table = malloc(sizeof(struct tilted_tw_table));
    new_table->next = NULL;
    new_table->buffer_empty = 1;
    new_table->freq = add;
    new_table->starting_batch = starting_batch;
    new_table->ending_batch = ending_batch;
    return new_table;
}

tilted_tw_table insert_batch(tilted_tw_table table, int starting_batch, int ending_batch, float add)
{

    if(table == NULL)
    {
        return create_new_tilted_tw_table(starting_batch, ending_batch, add);
    }

    if(table->starting_batch == starting_batch)
    {
        table->freq += add;
        return table;
    }

    if(table->buffer_empty == 1)
    {

        //no need for further propagation
        //just fill the buffer

        table->buffer_empty = 0;
        table->buffer_starting_batch = table->starting_batch;
        table->buffer_ending_batch = table->ending_batch;
        table->buffer_freq = table->freq;

        table->starting_batch = starting_batch;
        table->ending_batch = ending_batch;
        table->freq = add;

        return table;
    }
    else{

        //the buffer is combined with current contents and propagated forward
        //new node takes its place and buffer becomes empty

        int combined_tw_starting_batch = min(table->starting_batch, table->buffer_starting_batch);

        int combined_tw_ending_batch = min(table->ending_batch, table->buffer_ending_batch);

        float new_add = table->freq + table->buffer_freq;

        table->next = insert_batch(table->next, combined_tw_starting_batch, combined_tw_ending_batch, new_add);

        table->buffer_empty = 1;
        table->starting_batch = starting_batch;
        table->ending_batch = ending_batch;
        table->freq = add;
        return table;
    }
    assert(0);
}

void update_tilted_tw_table(pattern_node node, int batch_num, float add)
{
    node->table = insert_batch(node->table, batch_num, batch_num, add);
}



//////////////////////////////////////////////////////////////////////////////


pfpnode dfs(pattern_node current_node)
{

    float f = 0.0;
    tilted_tw_table curr_table = current_node->table;
    while(curr_table != NULL)
    {
        f += curr_table->freq;
        curr_table = curr_table->next;
    }
    if(f == 0 && current_node->data_item != -1)    return NULL;

    // printf("now at ");
    // print_node(current_node);

    pfpnode new_node = malloc(sizeof(struct pfp_node));
    new_node->next_similar = NULL;
    new_node->parent = NULL;
    new_node->item_list = NULL;
    new_node->children = NULL;
    new_node->touched = 0;
    new_node->freq = f;
    new_node->data_item = current_node->data_item;

    pattern_node_list current_child_ptr = current_node->children;
    pdata current_data_ptr = current_node->item_list;
    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        pdata this_data_item = current_data_ptr;

        pfpnode new_child = dfs(this_child);
        if(new_child != NULL)
            fp_insert_new_child(new_node, new_child, current_data_ptr);

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }

    return new_node;
}


pfptree get_fptree(patterntree tree)
{
    pfptree new_tree = malloc(sizeof(struct pfptree_node));
    new_tree->root = dfs(tree->root);
    fp_create_header_table(new_tree);
    return new_tree;
}

//////////////////////////////////////////////////////////////////////////////

void delete_tail_freq_table(tilted_tw_table t)
{
    if(t == NULL)    return;
    delete_tail_freq_table(t->next);
    free(t);
}


void tail_prune(pattern_node current_node)
{

    tilted_tw_table curr = current_node->table;
    if(curr == NULL)    return;

    tilted_tw_table prev = curr;
    curr = curr->next;
    float sum = prev->freq;
    while(curr != NULL)
    {

        sum += curr->freq;
        if(sum < MINSUP_FREQ && curr->freq < MINSUP_FREQ)    break;
        curr = curr->next;
    }

    delete_tail_freq_table(curr);
    prev->next = NULL;

    if(current_node->table->freq < MINSUP_FREQ)
    {
        delete_tail_freq_table(current_node->table);
        current_node->table = NULL;
    }

    pattern_node_list current_child_ptr = current_node->children;
    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        tail_prune(this_child);
        current_child_ptr = current_child_ptr->next;
    }
}
//////////////////////////////////////////////////////////////////////////////
///////////////////////////// Print Functions ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


void print_node(pattern_node node)
{
    if(node == NULL)    return;
    int c = 0;
    pdata d = node->item_list;
    while(d != NULL)
    {
        c++;
        d = d->next;
    }

    float f = 0;
    tilted_tw_table curr_table = node->table;
    while(curr_table != NULL)
    {
        f += curr_table->freq;
        curr_table = curr_table->next;
    }

    printf("data_item = %d, children = %d tot_freq = %lf\n", node->data_item, c, f);
}


void print_tree(pattern_node node)
{
    print_node(node);
    pattern_node_list curr_child_list = node->children;
    pdata curr_data = node->item_list;
    while(curr_data != NULL)
    {
        pattern_node this_child = curr_child_list->tree_node;
        printf("going to child %d %d\n", this_child->data_item, curr_data->data_item);
        print_tree(this_child);
        curr_child_list = curr_child_list->next;
        curr_data = curr_data->next;
    }
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////// Old FP-tree functions ///////////////////////////
//////////////////////////////////////////////////////////////////////////////


// this function fixes the next_similar variable also
void fp_create_header_table_helper(pfpnode root, pheader_table* h)
{
    //append this node to the corresponding list for this pdata item in the header table
    data_type this_data = root->data_item;

    pheader_table curr_header_node = *h;
    int found = 0;
    while(curr_header_node != NULL)
    {
        if(curr_header_node->data_item == this_data)
        {

            //append to the head of the linked list for this pdata item
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
        pheader_table new_entry = malloc(sizeof(struct pheader_table_node));
        new_entry->data_item = this_data;
        new_entry->first = root;
        new_entry->cnt = root->freq;
        new_entry->next = *h;
        *h = new_entry;
    }

    pfpnode_list current_child_ptr = root->children;

    while(current_child_ptr != NULL)
    {
        pfpnode this_child = current_child_ptr->tree_node;
        fp_create_header_table_helper(this_child, h);
        current_child_ptr = current_child_ptr->next;
    }
}


void fp_create_header_table(pfptree tree)
{
    if(tree == NULL)    return;
    tree->head_table = NULL;
    tree->root->parent = NULL;
    fp_create_header_table_helper(tree->root, &(tree->head_table));
}


void fp_insert_new_child(pfpnode current_node, pfpnode new_child, pdata d)
{

    new_child->parent = current_node;
    pdata new_data = malloc(sizeof(struct pdata_node));
    new_data->data_item = d->data_item;
    new_data->next = NULL;

    pfpnode_list new_list_node = malloc(sizeof(struct pfpnode_list_node));
    new_list_node->tree_node = new_child;
    new_list_node->next = NULL;

    new_list_node->next = current_node->children;
    current_node->children = new_list_node;
    new_data->next = current_node->item_list;
    current_node->item_list = new_data;
}

void fp_delete_data_node(pdata d)
{
    pdata temp;
    while(d){
        temp = d;
        d = d->next;
        free(temp);
        temp = NULL;
    }
}

pfptree fp_create_fptree()
{

    pfptree new_tree = malloc(sizeof(struct pfptree_node));
    if(new_tree == NULL)
    {
        printf("new_tree malloc failed\n");
    }

    pfpnode node = malloc(sizeof(struct pfp_node));
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

    new_tree->root = node;
    new_tree->head_table = NULL;
    return new_tree;
}


void fp_delete_tree_structure(pfpnode current_node)
{
    if(current_node == NULL)
        return;

    pfpnode_list current_child_ptr = current_node->children;
    pdata current_data_ptr = current_node->item_list;

    while(current_child_ptr != NULL)
    {

        pfpnode this_child = current_child_ptr->tree_node;
        fp_delete_tree_structure(this_child);
        free(this_child);
        this_child = NULL;

        pfpnode_list temp_list = current_child_ptr;
        current_child_ptr = current_child_ptr->next;
        free(temp_list);
        temp_list = NULL;

        pdata temp_data = current_data_ptr;
        current_data_ptr = current_data_ptr->next;
        free(temp_data);
        temp_data = NULL;
    }
}

void fp_delete_header_table(pheader_table h)
{
    if(h == NULL)    return;
    fp_delete_header_table(h->next);
    free(h);
    h = NULL;
}


void fp_delete_fptree(pfptree tree)
{
    if(tree == NULL)    return;
    fp_delete_tree_structure(tree->root);
    free(tree->root);
    tree->root = NULL;
    fp_delete_header_table(tree->head_table);
    free(tree);
    tree = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////// Old FP-tree mine functions //////////////////////////
//////////////////////////////////////////////////////////////////////////////


pfpnode fp_dfs(pfpnode node, data_type highest_priority_data_item)
{

    if(node->touched == 0)    return NULL;

    pfpnode new_node = malloc(sizeof(struct fp_node));
    new_node->children = NULL;
    new_node->item_list = NULL;
    new_node->touched = 0;
    new_node->freq = node->touched;
    new_node->data_item = node->data_item;
    new_node->next_similar = NULL;

    if(new_node->data_item == highest_priority_data_item)
    {
        node->touched = 0;
        return new_node;
    }

    pfpnode_list temp_child_list = node->children;
    pdata temp_data = node->item_list;
    while(temp_child_list != NULL)
    {
        pfpnode this_child = temp_child_list->tree_node;
        if(this_child->touched > 0)
        {
            pfpnode new_child = fp_dfs(this_child, highest_priority_data_item);
            if(new_child == NULL)    continue;
            fp_insert_new_child(new_node, new_child, temp_data);
        }

        temp_child_list = temp_child_list->next;
        temp_data = temp_data->next;
    }

    node->touched = 0;
    return new_node;
}


pfptree fp_create_conditional_fp_tree(pfptree tree, data_type data_item, int minsup)
{
    pheader_table curr_head_table_node = tree->head_table;
    pfpnode node = NULL;
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

    // node is the link to successive fpnodes having pdata type 'data_item'
    // iterate through it and for each node in it, start from that node
    // and touch all nodes till the root
    // touched nodes are a means of identifying which nodes should be in coditional FP-tree

    while(node != NULL)
    {
        pfpnode temp = node;
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

    pfpnode cond_fptree = fp_dfs(tree->root, data_item);
    if(cond_fptree == NULL)    return NULL;

    // printf("cond_fptree\n");
    // print_tree(cond_fptree);
    // printf("\n");

    pfptree cond_tree = malloc(sizeof(struct fptree_node));
    cond_tree->root = cond_fptree;
    cond_tree->head_table = NULL;
    fp_create_header_table(cond_tree);
    return cond_tree;
}


void fp_mine_frequent_itemsets(pfptree tree, pdata sorted, pdata till_now, int tid, int pattern)
{
    if(tree == NULL)    return;

    // if(till_now == NULL)
    //     printf("\nentered new mine with till_now = NULL and sorted = %d\n", sorted->data_item);
    // else if(sorted == NULL)
    //     printf("\nentered new mine sorted = NULL\n");
    // else
    //     printf("\nentered new mine sorted = %d\n", sorted->data_item);

    pheader_table curr_header_node = tree->head_table;

    if(till_now != NULL)
    {
        // printf("till_now = ");
        pdata temp = till_now;

        // fp_print_data_node(till_now);

        //current itemset is not empty
        pdata last_item = till_now;

        while(last_item->next != NULL)
            last_item = last_item->next;

        while(curr_header_node != NULL)
        {
            if(curr_header_node->data_item == last_item->data_item)    break;
            curr_header_node = curr_header_node->next;
        }

        assert(curr_header_node != NULL);
        if((pattern == 0 && curr_header_node->cnt >= tid*MINSUP_SEMIFREQ)
                || (pattern == 1 && curr_header_node->cnt >= tid*MINSUP_FREQ))
        {
            //frequent itemset
            FILE *fp;
            if(pattern == 0)
                fp = fopen("intermediate", "a");
            else
                fp = fopen("output", "a");

            int t=0, arr[DICT_SIZE];

            pdata temp = till_now;
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
                t--;
            }
            // if(pattern == 0)
            {
                fprintf(fp, " %lf", curr_header_node->cnt);
                // printf(" %d", curr_header_node->cnt);
            }
            // printf("\n");

            fprintf(fp, "\n");
            fclose(fp);
            // printf("printed to intermediate\n");
        }
    }
    // else{

    // }

    if(sorted == NULL)    return;

    //now check for supersets of this itemset by considering every next pdata item
    //in the sorted list
    pdata curr_data = sorted;
    while(curr_data != NULL)
    {

        pfptree cond_tree = fp_create_conditional_fp_tree(tree, curr_data->data_item,
                (pattern == 1) ? tid*MINSUP_FREQ : tid*MINSUP_SEMIFREQ);
        if(cond_tree == NULL)
        {
            // printf("skipped %d\n", curr_data->data_item);
            curr_data = curr_data->next;
            continue;
        }

        //append to front of current itemset
        pdata new_data = malloc(sizeof(struct pdata_node));
        new_data->data_item = curr_data->data_item;
        new_data->next = NULL;

        if(till_now == NULL)
            till_now = new_data;
        else{
            pdata temp = till_now;
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
            pdata temp_del = till_now->next;
            pdata prev = till_now;
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

void print_pdata_node(pdata d)
{
    printf("{");
    while(d)
    {
        printf("%d, ", d->data_item);
        d = d->next;
    }
    printf("}\n");
}