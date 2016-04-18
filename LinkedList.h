/***********************************************************************
*	Filename : LinkedList.h
*	Authors : Pedro Abreu
*			  Joao Borrego
*	Description: Header file for LinkedList functions
*
**********************************************************************/

#ifndef LinkedListHeader
#define LinkedListHeader

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

typedef struct LinkedListStruct LinkedList;

/******************************************************************************
 * insertUnsortedLinkedList()
 *
 * Arguments: LinkedList* first, Item
 * Returns: LinkedList* node
 * Side-Effects: creates a new Node with an Item in it
 *
 * Description: inserts a new node containing Item the beggining of the list, unsorted, 
 *              returning a pointer to the first node of the list
 *
 *****************************************************************************/
LinkedList * insertUnsortedLinkedList(LinkedList * first, Item this);

/******************************************************************************
 * getItemLinkedList()
 *
 * Arguments: LinkedList* node
 * Returns: Item (void*)
 * Side-Effects: none
 *
 * Description: returns the Item contained in the LinkedList* node argument
 *
 *****************************************************************************/
Item getItemLinkedList(LinkedList * node);

/******************************************************************************
 * getNextNodeLinkedList()
 *
 * Arguments: LinkedList* node
 * Returns: LinkedList* 
 * Side-Effects: none
 *
 * Description: returns a pointer to the next node in a list
 *
 *****************************************************************************/
LinkedList * getNextNodeLinkedList(LinkedList * node);
/******************************************************************************
 * freeLinkedList()
 *
 * Arguments: LinkedList* first, void (*freeItem)(Item, int), int x)
 * Returns: void
 * Side-Effects: Frees a LinkedList, all it's nodes and whatever they contain
 *
 * Description: Receives the pointer to the first node of the list and a function pointer
 *              to freeItems (together with it's arguments), and frees the LinkedList 
 *              and it's data
 *
 *****************************************************************************/
void freeLinkedList(LinkedList * first, void (*freeItem)(Item, int), int x);

#endif