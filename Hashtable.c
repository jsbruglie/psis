#include "Hashtable.h"

int kv_freeKvPair(void* kv){
	
	kv_pair* aux = (kv_pair*) kv;
	free(aux->value);
	free(aux);
	return 0;
}

kv_pair* kv_allocKvPair(uint32_t key, char* value, int value_length){
	
	kv_pair* new_kv = (kv_pair*) malloc(sizeof(kv_pair));
	new_kv->key = key;
	new_kv->value = (char*) malloc(sizeof(char) * value_length);
	memcpy(new_kv->value, value, value_length);
	new_kv->value_length = value_length;
	return new_kv;
}

Hashtable* createHashtable(int size){
	
	if(size < 1) 
		return NULL;

	Hashtable* new;
	int i;

	new = (Hashtable*) malloc(sizeof(Hashtable));
	if (new == NULL){
		perror("createHashtable - hashtable");
		return NULL;
	}

	new->table = (LinkedList**) malloc(sizeof(LinkedList*) * size); 
	if (new->table == NULL){
		perror("createHashtable - hashtable->table");
		return NULL;
	}

	for(i = 0; i < size;i ++){
		new->table[i] = NULL;
	}

	new->lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * size);

	for (i = 0; i <size; i++){
		pthread_mutex_init(&(new->lock[i]), NULL);
	}
	
	new->size = size;
	new->occupied = 0;
	return new;
}

// As seen in https://gist.github.com/badboy/6267743
int hash(int size, uint32_t key){
	uint32_t hashval = key * 2654435761;
	return hashval % size;
}

kv_pair* hashtableRead(Hashtable* _hashtable, uint32_t key){
	
	LinkedList* list;
	kv_pair* kv_aux;
	kv_pair* kv;

	if (_hashtable == NULL){
		return NULL;
	}

	int hashval = hash(_hashtable->size, key);

	pthread_mutex_lock(&(_hashtable->lock[hashval]));
	for(list = _hashtable->table[hashval]; list != NULL; list = getNextNodeLinkedList(list)){
		kv_aux = (kv_pair*) getItemLinkedList(list);
		if(kv_aux->key == key){
			kv = kv_allocKvPair(key, kv_aux->value, kv_aux->value_length);
			pthread_mutex_unlock(&(_hashtable->lock[hashval]));
			return kv;
		}
	}
	pthread_mutex_unlock(&(_hashtable->lock[hashval]));
	return NULL;
}	

int kv_pairCompare(void* a, void* b){
	kv_pair* _a = (kv_pair*) a;
	kv_pair* _b = (kv_pair*) b;
	if(_b == NULL || _a == NULL)
		return -1;
	if(_a->key ==_b->key){
		return 1;
	}else{
		return 0;
	}
}

int kv_keyCompare(void* a, void* b){
	kv_pair* _a = (kv_pair*) a;
	uint32_t* _b = (uint32_t*) b;
	if(*_b == 0 || _a == NULL)
		return -1;
	if(_a->key == *_b){
		return 1;
	}else{
		return 0;
	}
}

int hashtableWrite(Hashtable* _hashtable, uint32_t key, char* value, int value_length, int overwrite){
	
	if (_hashtable == NULL){
		return -1;
	}
	kv_pair* kv = kv_allocKvPair(key, value, value_length);

	int hashval = hash(_hashtable->size, kv->key);
	
	pthread_mutex_lock(&(_hashtable->lock[hashval]));

	if (_hashtable->table[hashval] == NULL){
		_hashtable->occupied++;
	}

	int ret = tryInsert(&(_hashtable->table[hashval]), (void*) kv, kv_pairCompare, kv_freeKvPair, overwrite);
	// If the key was already taken and no overwrite is allowed
	pthread_mutex_unlock(&(_hashtable->lock[hashval]));
	
	if (ret == -2){
		kv_freeKvPair(kv);
	}
	return ret;
}

int hashtableDelete(Hashtable* _hashtable, uint32_t key){
	
	int* key_ptr = &key;
	if (_hashtable == NULL){
		return -1;
	}
	int hashval = hash(_hashtable->size, key);
	
	pthread_mutex_lock(&(_hashtable->lock[hashval]));
	
	int ret = tryDelete(&(_hashtable->table[hashval]), (void*) key_ptr, kv_pairCompare, kv_freeKvPair);
	if (_hashtable->table[hashval] == NULL)
		_hashtable->occupied--;

	pthread_mutex_unlock(&(_hashtable->lock[hashval]));
	return ret;
}

void freeHashtable(Hashtable* _hashtable){
	
	int i;
	LinkedList *list;
	
	if(_hashtable == NULL)
		return;
	
	for(i = 0; i < _hashtable->size; i++){
		list = _hashtable->table[i];
		freeLinkedList(list, kv_freeKvPair);
	}

	free(_hashtable->table);	
	free(_hashtable->lock);
	free(_hashtable);
}

int writeBackupHashtable(Hashtable* _hashtable, char* filename){
	
	FILE* fp;
	LinkedList* list;
	kv_pair* temp_kv;
	int ret = 0;
	int i;

	fp = fopen(filename, "w");  	
	for (i = 0; i < _hashtable->size; i++){
		for(list = _hashtable->table[i]; list != NULL; list = getNextNodeLinkedList(list)){
			temp_kv = (kv_pair*) getItemLinkedList(list);
			if(fwrite((void*) &(temp_kv->key), sizeof(uint32_t), 1, fp) == 0){						// key
				ret = ERROR;
				break;
			}
			if(fwrite((void*) &(temp_kv->value_length), sizeof(int), 1, fp) == 0){ 				// value_length
				ret = ERROR;
				break;
			} 
			if(fwrite((void*) temp_kv->value, sizeof(char), temp_kv->value_length, fp) == 0){		// value
				ret = ERROR;
				break;
			}
		}
	}
	fclose(fp); 
	return ret;
}

Hashtable* restoreFromFile(char* filename, int size){

	FILE* fp;
	Hashtable* _hashtable;
	LinkedList* list;
	kv_pair* temp_kv;
	int ret = 0;

	uint32_t key;
	int value_length;
	char* value;

	fp = fopen(filename, "r");
	_hashtable = createHashtable(size);	
	if (fp == NULL){
		return _hashtable;  
	}
	
	while(!feof(fp)){		
		if(fread(&(key), sizeof(uint32_t), 1, fp) == 0){ 			// key
			ret = ERROR;
			break; 
		}
		if(fread(&(value_length), sizeof(int), 1, fp) == 0){		// value_length
			ret = ERROR;
			break;
		}		
		value = (char*) malloc(sizeof(char) * value_length);
		if(fread(value, sizeof(char), value_length, fp) == 0){		// value
			ret = ERROR;
			break;
		}
		hashtableWrite(_hashtable, key, value, value_length, 0);
		free(value);
	}
	fclose(fp);
	return _hashtable;
}

int lockHashtable(Hashtable* _hashtable){
	int i;
	if (_hashtable == NULL)
		return ERROR;
	for (i = 0; i < _hashtable->size; i++){
		 pthread_mutex_lock(&(_hashtable->lock[i]));	
	}
	return 0;
}		

int unlockHashtable(Hashtable* _hashtable){
	int i;
	if (_hashtable == NULL)
		return ERROR;
	for (i = 0; i < _hashtable->size; i++){
		 pthread_mutex_unlock(&(_hashtable->lock[i]));	
	}
}
	
// DEBUG
void printHashtable(Hashtable* _hashtable){

	int i = 0;
	LinkedList* list;
	kv_pair* temp_kv;
	
	printf("Current Hashtable:\n");
	for (i = 0; i < _hashtable->size; i++){
		printf("%d - ", i);
		pthread_mutex_lock(&(_hashtable->lock[i]));
		for(list = _hashtable->table[i]; list != NULL; list = getNextNodeLinkedList(list)){
			temp_kv = (kv_pair*) getItemLinkedList(list);
			printf("%d->%s ", temp_kv->key, (char*) temp_kv->value);
		}
		pthread_mutex_unlock(&(_hashtable->lock[i]));
		printf("\n");
	}
}
