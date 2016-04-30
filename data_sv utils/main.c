#include "Hashtable.h"

int main(int argc, char const *argv[])
{
	Hashtable* h;
	unsigned long int size = 10;
	int a,b,c,d,e,f,g;
	
	h = createHashtable(size);
	
	a = hashtableWrite(h, 1, "a", (strlen("a") + 1), 0);
	b = hashtableWrite(h, 2, "b", (strlen("b") + 1), 0);
	c = hashtableWrite(h, 1, "c", (strlen("c") + 1), 0);
	d = hashtableWrite(h, 1, "d", (strlen("d") + 1), 1);
	e = hashtableWrite(h, 987, "e", (strlen("e") + 1), 1);
	g = hashtableWrite(h, 7, "g", (strlen("g") + 1), 1);
	printHashtable(h);
	hashtableDelete(h, 7);
	printHashtable(h);
	hashtableDelete(h,2);
	printHashtable(h);

	kv_pair* kv = hashtableRead(h, 1);
	if (kv != NULL){
		printf("Found key:%d value:%s size:%d\n", kv->key, kv->value, kv->value_length);
		kv_freeKvPair(kv);		
	}
	printf("a:%d b:%d c:%d d:%d e:%d g:%d\n", a,b,c,d,e,g);
	freeHashtable(h);
	
	return 0;
}
