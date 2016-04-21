#ifndef HahstableHeader
#define HashtableHeader

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

#include "LinkedList.h"


typedef struct _kv_pair{
	uint32_t key;
	char* value;
} kv_pair;

int kv_freeKvPair(void* kv);
kv_pair* kv_allocKvPair(uint32_t key, char* value, int value_length);

typedef struct HashtableStruct
{
  LinkedList** table;
  unsigned long int size;
  int occupied;
}Hashtable;

Hashtable* createHashtable(unsigned long int size);
int hash(Hashtable* _hashtable, uint32_t key);
char* hashtableRead(Hashtable** _hashtable, uint32_t key); 
int hashtableWrite(Hashtable* _hashtable, kv_pair* pair);


#endif