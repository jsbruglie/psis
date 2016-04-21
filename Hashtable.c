#include "Hashtable.h"

int kv_freeKvPair(void* kv){
	kv_pair* aux = (kv_pair*) kv;
	free(aux->value);
	free(aux);
	return 0;
}

kv_pair* kv_allocKvPair(uint32_t key, char* value, int value_length){
	kv_pair* new_kv = malloc(sizeof(kv_pair));
	new_kv->key = key;
	new_kv->value = malloc((sizeof(char) * value_length) + 1); /* value_length+1 ('\0') */
	strcpy(new_kv->value, value);
	new_kv->value[value_length] = '\0';
	return new_kv;
}

Hashtable* createHashtable(unsigned long int size){
	if(size<1) //DOn't be stupid
		return NULL;
	Hashtable* new_table; //Create new pointer to table
	new_table = malloc(sizeof(Hashtable));
	new_table->table = malloc(sizeof(LinkedList*) * size); //Create all the lists
	int i;
	for(i=0;i<size;i++){ //Initialize all the lists
		new_table->table[i]=NULL;
	}
	new_table->size = size; //This might be redundant
	new_table->occupied = 0;
	return new_table;
}

int hash(Hashtable* _hashtable, uint32_t key){ //crappy hash
	uint32_t hashval = key*(key+3)*0.5;
	return hashval%(_hashtable->size);
}

char* hashtableRead(Hashtable** _hashtable, uint32_t key){
	LinkedList* p;
	kv_pair* temp_kv;
	Hashtable* localtable = * _hashtable;

	int hashval = hash(localtable, key);
	printf("Hash is %d \n", hashval);
	p = localtable->table[hashval];
	temp_kv = (kv_pair*) getItemLinkedList(p);
	printf("Values is %s\n ", temp_kv->value);

	for(p = localtable->table[hashval]; p != NULL; p = getNextNodeLinkedList(p)){
		temp_kv = (kv_pair*) getItemLinkedList(p);
		if(temp_kv->key == key){
			return temp_kv->value;
		}
	}
	return NULL;
}	

int hashtableWrite(Hashtable* _hashtable, kv_pair* pair){

	if(hashtableRead(&_hashtable, pair->key) != NULL){
		printf("value already exists\n");
		return -1;
	}
	int hashval = hash(_hashtable, pair->key);
	_hashtable->table[hashval]=insertUnsortedLinkedList(_hashtable->table[hashval], pair);
	kv_pair* new_kv;
	new_kv = (kv_pair*) getItemLinkedList(_hashtable->table[hashval]);
	printf("Succesful insertion of kv pair: %d %s with hash %d\n", new_kv->key, new_kv->value, hashval);
	return 1;
}