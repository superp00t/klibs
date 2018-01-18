#include <stdint.h>
#include <stdio.h>

#ifndef DEBUG_H__
#define DEBUG_H__

#ifdef KDEBUG
void kdebug_bytes (const char *name, uint8_t *array, int size);
#define KDEBUGF(...) printf(__VA_ARGS__)
#define KDEBUG_BYTES kdebug_bytes

#else
#define KDEBUGF(...)
#define KDEBUG_BYTES(...)
#endif


#endif