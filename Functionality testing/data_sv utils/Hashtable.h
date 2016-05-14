#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "LinkedList.h"
#define ERROR -1

#define MAX_BUFFER 1024

typedef struct _kv_pair{
	uint32_t key;
	char* value;
	int value_length;
} kv_pair;

typedef struct HashtableStruct{
	LinkedList** table;
	int size;
	pthread_mutex_t* lock;
	int occupied;
}Hashtable;

// kv_pair related functions
kv_pair* kv_allocKvPair(uint32_t key, char* value, int value_length);
int kv_freeKvPair(void* kv);
int kv_pairCompare(void* a, void* b);
int kv_keyCompare(void* a, void* b);

// Hashtable related functions
Hashtable* createHashtable(int size);
int hash(int size, uint32_t key);
kv_pair* hashtableRead(Hashtable* _hashtable, uint32_t key); 
int hashtableWrite(Hashtable* _hashtable, uint32_t key, char* value, int value_length, int overwrite);
int hashtableDelete(Hashtable* _hashtable, uint32_t key);
void freeHashtable(Hashtable* _hashtable);

// backup and restore functions
int writeBackupHashtable(Hashtable* _hashtable, char* filename);
Hashtable* restoreFromFile(char* filename, int size);

// generic use mutex lock and unlock functions
int lockHashtable(Hashtable* _hashtable);
int unlockHashtable(Hashtable* _hashtable);

// DEBUG
void printHashtable(Hashtable* _hashtable);
