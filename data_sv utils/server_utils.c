#include "server_utils.h"

int processRequest(int client_fd, message m, Hashtable* _hashtable){

	int ret;
	if(m.flag == READ) { // READ
		if((ret = sv_read(client_fd, m, _hashtable)) != 0){
			if (ret == 1){
				perror("processRequest - sv_read - Not found: ");	
			}else{
				perror("processRequest - sv_read: ");
			}
			return ret;
		}

	}else if(m.flag == WRITE || m.flag == OVERWRITE){ // WRITE
		if( (ret = sv_write(client_fd, m, _hashtable)) != 0){
			if(ret == -2){
				perror("processRequest - sv_write - Overwrite not allowed: ");
			}
			else{
				perror("processRequest - sv_write: ");
			}
			return ret;
		}
		
	}else if(m.flag == DELETE){ // DELETE 
		if (sv_delete(client_fd, m, _hashtable) != 0){
			perror("processRequest - sv_delete: ");
			return -1;
		}
	}
	return 0;
}

int sv_write(int client_fd, message m, Hashtable* _hashtable){
	
	int nbytes;
	int ret = 0;
	
	message answer;
	answer.key = -1;
	
	uint32_t key = m.key;
	int value_length = m.value_length;
	// TODO - check if size is negative	for sanity
	if (value_length < 0){
		answer.flag = ERROR;
    	send(client_fd, &m, sizeof(message), 0);
		return ERROR;
	}	

	int overwrite = (m.flag == OVERWRITE);  
	
	if (overwrite == 0){
		if (hashtableRead(_hashtable, key) != NULL){
			answer.flag = OVR_ERROR;
			send(client_fd, &m, sizeof(message), 0);
			return OVR_ERROR;
		}
	}

	answer.key = -1;
	answer.flag = RECEIVED;
	nbytes = send(client_fd, &answer, sizeof(message), 0);	
	
	char buffer[MAX_SIZE];
	char* value = malloc(sizeof(char)*value_length);
	
	nbytes = recv(client_fd, buffer, value_length, 0); 	
	memcpy(value, buffer, value_length);
	
	if(hashtableWrite(_hashtable, key, value, value_length, overwrite) == 0){
		answer.flag = WRITTEN;
		answer.key = value_length;
	}else{
		answer.flag = ERROR;
		answer.key = 0;	
		ret = ERROR;
	}	
	nbytes = send(client_fd, &answer, sizeof(message), 0);
	return ret;
}

/*
int sv_read(int new_fd, message m, Hashtable** _hashtable){
	
	int nbytes;

	printf("Read access requested for key %d\n", m.key);
	printf("Found in the hash: %s\n", hashtableRead(_hashtable, m.key));
	//Send a message back to the client saying it doesn't exist
	if(hashtableRead(_hashtable, m.key)==NULL){
		strcpy(m.value, "This value does not exist in the hash");
		m.value[MAX_LEN - 1] = '\0'; 	
		nbytes = send(new_fd, &m, sizeof(message), 0); // This is pretty messy
		return nbytes;	
	}
	//Send a message back to the client saying it exists
	strcpy(m.value, hashtableRead(_hashtable, m.key));
	printf("Found value %s\n", m.value);
	m.value[MAX_LEN - 1] = '\0'; 	
	nbytes = send(new_fd, &m, sizeof(message), 0); // This is pretty messy
	return nbytes;
}
*/
