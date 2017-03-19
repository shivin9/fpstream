#include "def.h"

void delete_qstack(QStack* qstack);
void insert(hlink head, char* val); /* insert val in the dict node*/
char* map(int *state, int len); /* convert the array to string*/
int hash(char* key); /* hash value of a string*/
void save(dict htable, char* key); /* save the string in dict*/
int find(dict htable, char* key); /* check out if val is there in dict*/
void push(QStack* l, sfnode v); /* push a node in the qstack*/
sfnode get(QStack* l); /* get a node in LIFO manner*/
sfnode pop(QStack* l); /* get a node in FIFO manner*/
QStack* createQStack(); /* create a qstack*/