#include "LinkedList.h"

struct LinkedListStruct
{
  message this;
  LinkedList * next;
};

void freeLinkedList(LinkedList * first, void (*freemessage)(message, int), int x)
{
  LinkedList * next;
  LinkedList * aux;

  for(aux = first; aux != NULL; aux = next) 
  {
    next = aux->next;
    freemessage(aux->this, x);
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

message getMessageLinkedList(LinkedList * node)
{
  return node->this;
}

LinkedList * insertUnsortedLinkedList(LinkedList * first, message this)
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