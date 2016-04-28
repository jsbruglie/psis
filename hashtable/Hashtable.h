#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <pthread.h>

#include "LinkedList.h"

typedef struct _kv_pair{
	uint32_t key;
	char* value;
	int value_length;
} kv_pair;

typedef struct HashtableStruct{
	LinkedList** table;
	unsigned long int size;
	pthread_mutex_t* lock;
	int occupied;
}Hashtable;

// kv_pair related functions
kv_pair* kv_allocKvPair(uint32_t key, char* value, int value_length);
int kv_freeKvPair(void* kv);
int kv_pairCompare(void* a, void* b);
int kv_keyCompare(void* a, void* b);

// Hashtable related functions
Hashtable* createHashtable(unsigned long int size);
int hash(unsigned long int size, uint32_t key);
kv_pair* hashtableRead(Hashtable* _hashtable, uint32_t key); 
int hashtableWrite(Hashtable* _hashtable, uint32_t key, char* value, int value_length, int overwrite);
int hashtableDelete(Hashtable* _hashtable, uint32_t key);
void freeHashtable(Hashtable* _hashtable);

// Debug
void printHashtable(Hashtable* _hashtable);