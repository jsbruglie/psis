#include "LinkedList.h"

struct LinkedListStruct
{
	Item this;
	LinkedList * next;
};

LinkedList* getNextNodeLinkedList(LinkedList* node)
{
	if(node == NULL){
		return NULL;
	}else{
		return node->next;
	}
}

Item getItemLinkedList(LinkedList* node)
{
	if(node == NULL){
	 return NULL;
	}
	return node->this;
}

LinkedList* insertUnsortedLinkedList(LinkedList* first, Item this)
{
	LinkedList* new;

	new = (LinkedList*) malloc(sizeof(LinkedList));
	if(new == NULL){
		perror("LinkedList - insertUnsortedLinkedList: ");
		exit(1);
	}
	new->this = this;
	new->next = first;

	return new;
}

int freeLinkedList(LinkedList* first, int (*freeItem)(Item)){
	LinkedList * next;
	LinkedList * aux;

	for(aux = first; aux != NULL; aux = next) {
		next = aux->next;
		freeItem(aux->this);
		free(aux);
	}
	return 0;
}

int tryDelete(LinkedList** first, Item this, int (*compareItem)(Item, Item), int (*freeItem)(Item)){
	
	LinkedList* aux, *prev;
	LinkedList* local = *(first);

	for(aux = local; aux != NULL; aux = getNextNodeLinkedList(aux)){
		if(compareItem((Item) this, aux->this)){
			// Delete at the beginning of the list
			if (aux == local){
				*(first) = local->next;
			}
			else {
				prev->next = aux->next;
				*(first) = local; 
			}
			freeItem(aux->this);
			free(aux);
			return 0;
		}
		prev = aux;
	}
	return -1;
}

int tryInsert(LinkedList** first, Item this, int (*compareItem)(Item, Item), int (*freeItem)(Item), int overwrite){
	
	LinkedList* aux;
	LinkedList* local = *(first);

	for(aux = local; aux != NULL; aux = getNextNodeLinkedList(aux)){
		if(compareItem((Item) this, aux->this)){
			if (overwrite){
				freeItem(aux->this);
				aux->this = this;
				*(first) = local;
				return 1;
			}
			else{
				return -2;
			}
		}
	}

	local = insertUnsortedLinkedList(local, this);
	*(first) = local;
	return 0;
}

