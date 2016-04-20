/*For testing if the library works only*/

#include "psiskv.h"

int main(){
	/* DEBUG COMMUNICATION */
	char* val = "potato";
	char read_val[MAX_LEN];
	uint32_t key = 30;

	printf("Connecting to server\n");
	int kv_descriptor = kv_connect("127.0.0.1",9999); // This descriptor is what the API will use from now on

	printf("Writing %s to key %d\n", val, key);
 	kv_write(kv_descriptor, key, val, strlen(val));
 	
 	printf("Reading from server\n");
 	kv_read(kv_descriptor, key, read_val, strlen(read_val));
 	
 	printf("Received %s  \n", read_val);
 	
 	printf("Closing connection to server\n");
 	kv_close(kv_descriptor);

	exit(0);
}
