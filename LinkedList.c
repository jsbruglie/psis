#include "LinkedList.h"

struct LinkedListStruct
{
  Item this;
  LinkedList * next;
};

int freeLinkedList(LinkedList * first, int (*freeItem)(Item)){
  LinkedList * next;
  LinkedList * aux;

  for(aux = first; aux != NULL; aux = next) {
    next = aux->next;
    freeItem(aux->this);
    free(aux);
  }
  return 0;
}

LinkedList * getNextNodeLinkedList(LinkedList * node)
{
  if(node == NULL){
  	return NULL;
  }else{
  	return node->next;
  }
}

Item getItemLinkedList(LinkedList * node)
{
  if(node == NULL){
	 return NULL;
  }
  return node->this;
}

LinkedList * insertUnsortedLinkedList(LinkedList * first, Item this)
{
  LinkedList *new;

  new = (LinkedList *) malloc(sizeof(LinkedList));
  if(new == NULL){
    fprintf(stderr, "Error : Could not allocate memory for the list.\n");
    exit(1);
  }
  new->this = this;
  new->next = first;

  return new;
}