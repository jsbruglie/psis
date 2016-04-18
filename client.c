/*For testing if the library works only*/

#include "psiskv.h"
// #include "config.h"

int main(){
	char* val = "potato";
	char readval[200]; //This should be malloced
	uint32_t key = 30;

	int descriptor = kv_connect("127.0.0.1",9999);
	//This descriptor is what the API will use from now on

 	kv_write(descriptor, key, val, strlen(val));
 	kv_read(descriptor, key, readval, strlen(readval));
 	printf("Received %s  \n", readval);
 	kv_close(descriptor);

	exit(0);
}
