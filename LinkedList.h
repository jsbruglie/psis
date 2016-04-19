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
#include "psiskv.h"

typedef struct LinkedListStruct LinkedList;

/******************************************************************************
 * insertUnsortedLinkedList()
 *
 * Arguments: LinkedList* first, message
 * Returns: LinkedList* node
 * Side-Effects: creates a new Node with an message in it
 *
 * Description: inserts a new node containing message the beggining of the list, unsorted, 
 *              returning a pointer to the first node of the list
 *
 *****************************************************************************/
LinkedList * insertUnsortedLinkedList(LinkedList * first, message this);

/******************************************************************************
 * getmessageLinkedList()
 *
 * Arguments: LinkedList* node
 * Returns: message (void*)
 * Side-Effects: none
 *
 * Description: returns the message contained in the LinkedList* node argument
 *
 *****************************************************************************/
message getMessageLinkedList(LinkedList * node);

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
 * Arguments: LinkedList* first, void (*freemessage)(message, int), int x)
 * Returns: void
 * Side-Effects: Frees a LinkedList, all it's nodes and whatever they contain
 *
 * Description: Receives the pointer to the first node of the list and a function pointer
 *              to freemessages (together with it's arguments), and frees the LinkedList 
 *              and it's data
 *
 *****************************************************************************/
void freeLinkedList(LinkedList * first, void (*freemessage)(message, int), int x);

#endif