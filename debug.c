#include "debug.h"

void kdebug_bytes (const char *name, uint8_t *array, int size)
{
  printf("%s = [ %d", name, array[0]);
  for(int i = 1; i < size; i++)
  {
    printf(", %d", array[i]);
  }
  printf(" ]\n");
}