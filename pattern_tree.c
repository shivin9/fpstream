#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include "pattern_tree.h"

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

void delete_pattern_node(pattern_node pnode){
    pattern_node_list child = pnode->children, prev;
    while(child){
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

void create_and_insert_new_child(pattern_node current_node, data d)
{

    pattern_node new_node = create_new_pattern_node(d->data_item);

    pattern_node_list new_list_node = malloc(sizeof(struct pattern_node_list_node));
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


////////////////////////////////////////////////////////////////////////////////

pattern_node insert_itemset_helper(pattern_node current_node, data d, int batch_num, float add)
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
    //if the next data item has already occurred earlier, go along that child
    //otherwise make a new child

    pattern_node_list current_child_ptr = current_node->children;
    data current_data_ptr = current_node->item_list;

    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

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

    //data item has to be inserted as new child
    assert(current_data_ptr == NULL);
    assert(current_child_ptr == NULL);

    create_and_insert_new_child(current_node, d);

    current_child_ptr = current_node->children;
    current_data_ptr = current_node->item_list;
    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

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


patterntree insert_itemset(patterntree tree, data d, int batch_num, float add)
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


fpnode dfs(pattern_node current_node)
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

    fpnode new_node = malloc(sizeof(struct fp_node));
    new_node->next_similar = NULL;
    new_node->parent = NULL;
    new_node->item_list = NULL;
    new_node->children = NULL;
    new_node->touched = 0;
    new_node->freq = f;
    new_node->data_item = current_node->data_item;

    pattern_node_list current_child_ptr = current_node->children;
    data current_data_ptr = current_node->item_list;
    while(current_child_ptr != NULL)
    {

        pattern_node this_child = current_child_ptr->tree_node;
        data this_data_item = current_data_ptr;

        fpnode new_child = dfs(this_child);
        if(new_child != NULL)
            fp_insert_new_child(new_node, new_child, current_data_ptr);

        current_child_ptr = current_child_ptr->next;
        current_data_ptr = current_data_ptr->next;
    }

    return new_node;
}



fptree get_fptree(patterntree tree)
{
    fptree new_tree = malloc(sizeof(struct fptree_node));
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


void print_node(pattern_node node)
{
    if(node == NULL)    return;
    int c = 0;
    data d = node->item_list;
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
    data curr_data = node->item_list;
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


