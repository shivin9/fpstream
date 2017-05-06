#include "qstack.h"

void delete_qstack(QStack* qstack)
{
    slink curr = qstack->head, next;
    if(curr != NULL)
    {
        while(curr)
        {
            next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(qstack);
    return;
}

/* inserting the string in the corresponding linked list of the hash table*/
void insert(hlink head, char* node)
{
    while(head->next != NULL)
        head = head->next;

    head->next = (hlink) malloc(sizeof(hnode));
    head->next->sig = node;
    head->next->next = NULL;
    return;
}


/* mapping the state matrix to a string*/
char* map(int *state, int len)
{
    int i;
    char *node = (char*) calloc(len, sizeof(char));
    for(i = 0; i < len; i++)
        node[i] = '0' + (state[i]);
    return node;
}


/* to hash the game state which is in the form of a string*/
int hash(char* key)
{
    int sum = 0, i = 0, len = strlen(key);
    for(i = 0; i < len; i++)
        sum += (key[i] - '0')*(i+1)*(sum+1);

    sum = abs(sum) % HSIZE;
    return sum;
}


/* saving stuff in the hash table*/
void save(dict htable, char* key)
{
    int sum = 0, i, row;
    row = hash(key);
    hlink toput = htable->table[row];
    htable->size += 1;
    insert(toput, key);
    return;
}


/* find function for the hash table*/
int find(dict htable, char* key)
{
    int sum = 0, i, row;
    row = hash(key);
    hlink toput = htable->table[row]->next;
    hlink prev;
    while(toput)
    {
        if(!strcmp(toput->sig, key))
            return 1;
        prev = toput;
        toput = toput->next;
    }
    return 0;
}


/* to push a node onto the QStack.*/
void push(QStack* l, sfnode v)
{
    if(v == NULL)
        return;
    
    slink temp = l->head;
    (l->size)++;

    if(temp == NULL) /* empty stack*/
    {
        slink new = (slink) malloc(sizeof(snode));
        new->node = v;
        new->next = NULL;
        new->prev = NULL;
        l->head = new;
        l->tail = new;
    }
    else
    {
        temp = l->tail;
        slink new = (slink) malloc(sizeof(snode));
        temp->next = new;
        l->tail = new;
        new->node = v;
        new->next = NULL;
        new->prev = temp;
    }
    return;
}


/* get() function to get an element from the QStack in LIFO manner. 
   It deletes the element from the qstack subsequently*/
sfnode get(QStack* l)
{
    slink curr = l->head;
    sfnode v;
    if(curr == NULL)
        return;

    else
    {
        v = curr->node;
        l->head = curr->next;
        if(curr->next)
            curr->next->prev = l->head;
        l->size--;
        free(curr);
    }
    return v;
}


/* pop() function to get an element from the QStack in FIFO manner*/
sfnode pop(QStack* l)
{
    slink curr = l->tail;
    sfnode v;
    if(curr == NULL)
    {
        // printf("no nodes to remove");
        return NULL;
    }
    else if(curr == l->head) /* only one element is present in the stack*/
    {
        v = curr->node;
        l->head = NULL;
        curr->prev = NULL;
        l->size = 0;
    }
    else
    {
        v = curr->node;
        (curr->prev)->next = NULL;
        l->tail = curr->prev;
        curr->prev = NULL;
        l->size--;
    }
    free(curr);
    return v;
}

/* Initializes the QStack*/
QStack* createQStack()
{
    QStack* l = (QStack*) malloc(sizeof(QStack));
    l->size = 0;
    l->head = NULL;
    return l;
}
