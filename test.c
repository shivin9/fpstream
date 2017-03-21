#include<stdio.h>

typedef struct sf_node* sfnode;
typedef struct header_table_node* header_table;

struct sf_node{
    sfnode* children;
    int* item_list;
    header_table hnode;
    struct sf_node* parent;
};

int main()
{
	sfnode temp = calloc(1, sizeof(struct sf_node));
	temp->children = calloc(100, sizeof(sfnode));
	temp->item_list = calloc(10, sizeof(int));
	printf("sizeof sfnode = %ld\n", sizeof(temp));
	printf("sizeof children = %ld\n", 100*sizeof(temp->children));
	printf("sizeof itemlist = %ld\n", 100*sizeof(temp->item_list));
	printf("sizeof parent = %ld\n", sizeof(temp->parent));
	return 0;
}
