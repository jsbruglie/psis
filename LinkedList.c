#include "LinkedList.h"

struct LinkedListStruct
{
  Item this;
  LinkedList * next;
};

void freeLinkedList(LinkedList * first, void (*freeItem)(Item, int), int x)
{
  LinkedList * next;
  LinkedList * aux;

  for(aux = first; aux != NULL; aux = next) 
  {
    next = aux->next;
    freeItem(aux->this, x);
    free(aux);
  }
  return;
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
    return first;
  }
  new->this = this;
  new->next = first;

  return new;
}