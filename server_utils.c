#include "server_utils.h"

// Verbose
//#define VERBOSE 1 // Uncomment this line for verbose terminal output

#ifdef VERBOSE
#define debugPrint(str){printf(str);}
#define debugPrint1(str,arg){printf(str,arg);}
#else
#define debugPrint(str)
#define debugPrint1(str,arg)
#endif

int processRequest(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock){

	int ret = 0;
	if(m.flag == READ) { // READ
		if((ret = sv_read(client_fd, m, _hashtable)) != 0){
			if (ret == 1){
				debugPrint("[DS - pR]\tprocessRequest - sv_read - Key not found\n"); // DEBUG	
			}else{
				debugPrint("[DS - pR]\tprocessRequest - sv_read\n"); // DEBUG
			}
	}

	}else if(m.flag == WRITE || m.flag == OVERWRITE){ // WRITE
		if( (ret = sv_write(client_fd, m, _hashtable, log_fp, log_lock)) != 0){
			if(ret == OVR_ERROR ){
				debugPrint("[DS - pR]\tprocessRequest - sv_write - Overwrite not allowed.\n"); // DEBUG
			}
			else{
				debugPrint("[DS - pR]\tprocessRequest - sv_write\n"); // DEBUG
			}
		}
	}else if(m.flag == DELETE){ // DELETE 
		if (sv_delete(client_fd, m, _hashtable, log_fp, log_lock) != 0){
			debugPrint("[DS - pR]\tprocessRequest - sv_delete\n"); // DEBUG
			ret = -1;
		}
	}
	return ret;
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
	if(value_length < 0){
		ret = ERROR;
	}
	else if((kv = hashtableRead(_hashtable, key)) == NULL){
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
	char* value ;
	
	uint32_t key = m.key;
	int value_length = m.value_length;
	int overwrite = (m.flag == OVERWRITE);
	if (value_length < 0){								
		answer.flag = ERROR;
    	send(client_fd, &answer, sizeof(message), 0);
		return ERROR;
	}	
	
	value = malloc(sizeof(char) * value_length);
	
	// Receive the actual value
	nbytes = recv(client_fd, value, value_length, 0);
	memcpy(value, value, value_length);
		
	if((ret = hashtableWrite(_hashtable, key, value, value_length, overwrite)) == 0){
		answer.key = value_length;
	}else{
		answer.key = 0;	
	}
	answer.flag = ret; // ret will be OK upon sucess, OVR_ERROR in case of overwrite error or ERROR

	nbytes = send(client_fd, &answer, sizeof(message), 0);
	debugPrint1("\tInserted %d -", key);
	debugPrint1(" %s in the data structure.\n", value);	

	if (ret != ERROR){
		logEntry(log_fp, log_lock, WRITE, key,value, value_length);
	}	
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
	if (ret != ERROR){
		logEntry(log_fp, log_lock, DELETE, key, NULL, -1);
	}
	return ret;
}

int logEntry(FILE* log_fp, pthread_mutex_t* log_lock, int flag, int key, char* value, int value_length){
	
	int ret = 0;
	pthread_mutex_lock(log_lock);
	if(fwrite((void*) &flag, sizeof(int), 1, log_fp) == 0){					// flag
		ret = ERROR;
	}
	if(fwrite((void*) &key, sizeof(uint32_t), 1, log_fp) == 0){				// key 
		ret = ERROR;
	}
	if (flag == WRITE){
		if(fwrite((void*) &value_length, sizeof(int), 1, log_fp) == 0){		// value_length
			ret = ERROR;
		}
		if(fwrite((void*) value, sizeof(char), value_length, log_fp) == 0){	// value 
			ret = ERROR;
		}
	}
	fflush(log_fp);
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
			if(fread(&(flag), sizeof(int), 1, fp) == 0){ 				// flag	
				ret = ERROR;
				break; 
			}
			if(fread(&(key), sizeof(int), 1, fp) == 0){					// key
				ret = ERROR;
				break;
			}	
			if (flag == WRITE){
				if(fread(&(value_length), sizeof(int), 1, fp) == 0){	// value_length
					ret = ERROR;
					break;
				}	
				value = (char*) malloc(sizeof(char) * value_length);
				if(fread(value, sizeof(char), value_length, fp) == 0){	// value
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
