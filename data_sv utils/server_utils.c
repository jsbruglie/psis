#include "server_utils.h"

int processRequest(int client_fd, message m, Hashtable* _hashtable){

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
		if( (ret = sv_write(client_fd, m, _hashtable)) != 0){
			if(ret == OVR_ERROR ){
				printf("\tprocessRequest - sv_write - Overwrite not allowed.\n"); // DEBUG
			}
			else{
				printf("\tprocessRequest - sv_write\n"); // DEBUG
			}
			return ret;
		}
		
	}else if(m.flag == DELETE){ // DELETE 
		if (sv_delete(client_fd, m, _hashtable) != 0){
			printf("\tprocessRequest - sv_delete\n"); // DEBUG
			return -1;
		}
	}
	return 0;
}

int sv_write(int client_fd, message m, Hashtable* _hashtable){
	
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
	
	// This is an extra step and is a bit repetitive
	char* value = malloc(sizeof(char) * value_length);
	memcpy(value, buffer, value_length);

	// if(hashtableWrite(_hashtable, key, buffer, value_length, overwrite) == 0)
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
	free(value);
	return ret;
}

int sv_read(int client_fd, message m, Hashtable* _hashtable){
	
	int ret = 0;
	int nbytes;
	message answer;
	memset(&answer, 0, sizeof(message));
	kv_pair* kv;	
	char buffer[MAX_BUFFER];
	
	return 0;
}

int sv_delete(int client_fd, message m, Hashtable* _hashtable){
	
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

	return ret;
}
