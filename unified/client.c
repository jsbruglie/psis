/*For testing if the library works only*/

#include "psiskv.h"

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define READINGS 	2
#define INSERTIONS 	3
#define DELETIONS 	1
#define MAX_DELAY 	2

int main(){
	
	char* array[] = { "LongWord1", "Word2", "Word3", "Word4", "LongWord5", "Word6", "Word7"};
	int size = 7;

	int ret, r, i, o;
	srand(time(NULL));
	
	printf("Connecting to server\n");
	int kv_descriptor = kv_connect("127.0.0.1", 9999); // This descriptor is what the API will use from now on
	if (kv_descriptor == -1)
		exit(-1);

	for (i = 0; i < INSERTIONS; i++){
		r = rand() % size;
		o = rand() % 2;
 		ret = kv_write(kv_descriptor, r, array[r], (strlen(array[r]) + 1) * sizeof(char), o);
 		printf("Tried to write %d -> %s with permission %d. kv_write returned %d.\n", r, array[r], o, ret);
		sleep(r % MAX_DELAY);
	}
	for (i = 0; i < READINGS; i++){
		char read_val[MAX_BUFFER];
		r = rand() % size;
 		ret = kv_read(kv_descriptor, r, read_val, MAX_BUFFER);
 		printf("Tried to read %d. kv_read returned %d.", r, ret);
 		if (ret != -1){
 			printf("Found %s", read_val);
 		}
 		printf("\n");
 		sleep(r % MAX_DELAY);
	}
	for (i = 0; i < DELETIONS; i++){
 		r = rand() % size;
 		ret = kv_delete(kv_descriptor, r);
 		printf("Tried to delete %d. kv_delete returned %d.\n", r, ret);
 		sleep(r % MAX_DELAY);
 	}
 	
 	printf("Closing connection to server\n");
 	kv_close(kv_descriptor);

	exit(0);
}
