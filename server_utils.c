#include "server_utils.h"

int processRequest(int new_fd, message m, LinkedList** lp){

	if(m.flag == READ) { // READ
		if(sv_read(new_fd, m, lp) == -1){
			printf("Error reading\n");
			return -1;
		}

	}else if(m.flag == WRITE){ // WRITE
		if(sv_write(new_fd, m, lp) != 1){
			printf("Error reading\n");
			return -1;
		}
		
	}else if(m.flag == DELETE){ // DELETE 

	}
	return 0;
}

int sv_write(int new_fd, message m, LinkedList** lp){
	
	/* Insert into the list*/
	kv_pair* new_kv = kv_allocKvPair(m.key, m.value, strlen(m.value));
	
	/* So as to not override pointers */
	LinkedList* aux = *lp;
	aux = insertUnsortedLinkedList(aux, new_kv);
	
	/* Test that the item has indeed been inserted */
	new_kv = (kv_pair*) getItemLinkedList(aux);
	printf("Succesful insertion of kv pair: %d %s\n", new_kv->key, new_kv->value);
	
	*lp = aux;

	return 1;
}

int sv_read(int new_fd, message m, LinkedList** lp){
	
	int nbytes;
	LinkedList* aux = NULL;
	kv_pair* temp_kv;

	printf("Read access requested for key %d\n", m.key);
	if(m.key == 30){
		
		printf("Fetching a value for key %u...\n", m.key);

		for(aux = *lp; aux != NULL; aux = getNextNodeLinkedList(aux)){
			temp_kv = (kv_pair*) getItemLinkedList(aux);
			if(temp_kv->key == m.key){
				break;
			}
		}	

		strcpy(m.value, temp_kv->value);
		printf("Found value %s\n", m.value);
		m.value[MAX_LEN - 1] = '\0'; 	
		nbytes = send(new_fd, &m, sizeof(message), 0); /* This is pretty messy */
	}
	return nbytes;
}