#include "server_utils.h"

int processRequest(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock){

	int ret;
	if(m.flag == READ) { // READ
		if((ret = sv_read(client_fd, m, _hashtable)) != 0){
			if (ret == 1){
				printf("\tprocessRequest - sv_read - Key not found\n"); // DEBUG	
			}else{
				printf("\tprocessRequest - sv_read\n"); // DEBUG
			}
			return ret;
		}

	}else if(m.flag == WRITE || m.flag == OVERWRITE){ // WRITE
		if( (ret = sv_write(client_fd, m, _hashtable, log_fp, log_lock)) != 0){
			if(ret == OVR_ERROR ){
				printf("\tprocessRequest - sv_write - Overwrite not allowed.\n"); // DEBUG
			}
			else{
				printf("\tprocessRequest - sv_write\n"); // DEBUG
			}
			return ret;
		}
		
	}else if(m.flag == DELETE){ // DELETE 
		if (sv_delete(client_fd, m, _hashtable, log_fp, log_lock) != 0){
			printf("\tprocessRequest - sv_delete\n"); // DEBUG
			return -1;
		}
	}
	return 0;
}

int sv_read(int client_fd, message m, Hashtable* _hashtable){
	
	int ret = 0;
	int nbytes;
	message answer;
	memset(&answer, 0, sizeof(message));
	kv_pair* kv;	
	char* buffer;

	uint32_t key = m.key;
	int value_length = m.value_length;
	if ((kv = hashtableRead(_hashtable, key)) == NULL){
		answer.flag = ERROR;
		send(client_fd, &answer, sizeof(message), 0);
		ret = ERROR;
	}else{
		answer.flag = OK;
		answer.value_length = kv->value_length;
		send(client_fd, &answer, sizeof(message), 0);
		if (value_length > kv->value_length); // If the user has requested to read more than the value holds
			value_length = kv->value_length;
		buffer = malloc(sizeof(char) * value_length);
		memcpy(buffer, kv->value, value_length); 
		send(client_fd, buffer, answer.value_length, 0);
		kv_freeKvPair(kv);
		free(buffer);
	}

	return ret;
}

int sv_write(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock){
	
	int ret = 0;
	int nbytes;
	message answer;
	memset(&answer, 0, sizeof(message));
	kv_pair* kv;	
	char buffer[MAX_BUFFER];

	uint32_t key = m.key;
	int value_length = m.value_length;
	if (value_length < 0){						// TODO - or value_lenght > MAX_BUFFER
		answer.flag = ERROR;
    	send(client_fd, &m, sizeof(message), 0);
		return ERROR;
	}	

	int overwrite = (m.flag == OVERWRITE);  
	
	if (overwrite == 0){
		if ((kv = hashtableRead(_hashtable, key)) != NULL){
			answer.flag = OVR_ERROR;
			send(client_fd, &answer, sizeof(message), 0);
			kv_freeKvPair(kv);
			return OVR_ERROR;
		}
	}

	answer.flag = RECEIVED;
	nbytes = send(client_fd, &answer, sizeof(message), 0);	
	nbytes = recv(client_fd, buffer, value_length, 0);
	
	char* value = malloc(sizeof(char) * value_length);
	memcpy(value, buffer, value_length);

	if(hashtableWrite(_hashtable, key, value, value_length, overwrite) == 0){
		answer.flag = OK;
		answer.key = value_length;
	}else{
		answer.flag = ERROR;
		answer.key = 0;	
		ret = ERROR;
	}

	nbytes = send(client_fd, &answer, sizeof(message), 0);
	printf("\tInserted %d - %s in Hashtable.\n", key, value); // DEBUG
	logEntry(log_fp, log_lock, WRITE, key,value, value_length);
	free(value);
	return ret;
}

int sv_delete(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock){
	
	int ret = 0;
	int nbytes;
	message answer;
	memset(&answer, 0, sizeof(message));
	uint32_t key = m.key;

	if (hashtableDelete(_hashtable, key) != 0){
		answer.flag = ERROR;
		ret = ERROR;
	}
	else{
		answer.flag = OK;
	}
	nbytes = send(client_fd, &answer, sizeof(message), 0);
	logEntry(log_fp, log_lock, DELETE, key, NULL, -1);
	return ret;
}

int logEntry(FILE* log_fp, pthread_mutex_t* log_lock, int flag, int key, char* value, int value_length){
	
	pthread_mutex_lock(log_lock);
	if(fwrite((void*) &flag, sizeof(int), 1, log_fp) == 0){}		// flag
	if(fwrite((void*) &key, sizeof(uint32_t), 1, log_fp) == 0){}		// key 
	if(fwrite((void*) &value_length, sizeof(int), 1, log_fp) == 0){}	// value_length
	
	if (flag == WRITE){
		if(fwrite((void*) value, sizeof(char), value_length, log_fp) == 0){} 
	}
	pthread_mutex_unlock(log_lock);
	return 0;
}

FILE* processLogEntries(char* filename, Hashtable* _hashtable){
	
	FILE* fp;
	int ret = 0;
	uint32_t key;
	int flag;
	int value_length;
	char* value;

	fp = fopen(filename, "r");
	if (fp != NULL){
			
		while(!feof(fp)){		
			if(fread(&(flag), sizeof(int), 1, fp) == 0){ 			// flag	
				ret = ERROR;
				break; 
			}
			if(fread(&(key), sizeof(int), 1, fp) == 0){			// key
				ret = ERROR;
				break;
			}
			if(fread(&(value_length), sizeof(int), 1, fp) == 0){		// value_length
				ret = ERROR;
				break;
			}		
			
			if (flag == WRITE){
				value = (char*) malloc(sizeof(char) * value_length);
				if(fread(value, sizeof(char), value_length, fp) == 0){		// value
					ret = ERROR;
					break;
				}
				hashtableWrite(_hashtable, key, value, value_length, 0);
				free(value);
			}else if(flag == DELETE){
				hashtableDelete(_hashtable, key);
			}
		}
		fclose(fp);
	}
	return fopen(filename, "w");
}

FILE* eraseLog(FILE* log_fp, char* file_name){
	if (log_fp != NULL)
		fclose(log_fp);
	return fopen(file_name, "w");
}
