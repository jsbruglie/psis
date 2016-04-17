/*For testing if the library works only*/

#include "psiskv.h"
// #include "config.h"

int main(){
	char* val = "potato";
	uint32_t key = 30;

	int descriptor = kv_connect("127.0.0.1",9999);
	//This descriptor is what the API will use from now on

 	kv_write(descriptor, key, val, strlen(val));

 	kv_close(descriptor);

	exit(0);
}
