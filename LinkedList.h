#include <stdio.h>
#include <stdlib.h>

typedef void* Item;

typedef struct LinkedListStruct LinkedList;

Item getItemLinkedList(LinkedList* node);
LinkedList* getNextNodeLinkedList(LinkedList* node);

LinkedList* insertUnsortedLinkedList(LinkedList* first, Item this);
int freeLinkedList(LinkedList* first, int (*freeItem)(Item));

int tryInsert(LinkedList** first, Item this, int (*compareItem)(Item, Item), int (*freeItem)(Item), int overwrite);
int tryDelete(LinkedList** first, Item this, int (*compareItem)(Item, Item), int (*freeItem)(Item));