#include "Hashtable.h"

struct Hashtable
{
  LinkedList** table;
  int size;
  int occupied;
};