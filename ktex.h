#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <stdlib.h>
#include <string.h>

typedef struct {
#ifdef _WIN32
  HANDLE mutex;
#else
  pthread_mutex_t *mutex;
#endif
} ktex_t;

ktex_t *ktex_alloc();
void    ktex_destroy(ktex_t *k);
void    ktex_lock(ktex_t *k);
void    ktex_unlock(ktex_t *k);