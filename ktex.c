#include "ktex.h"

ktex_t *ktex_alloc()
{
  ktex_t *k = (ktex_t*)malloc(sizeof(ktex_t));
  memset(k, 0, sizeof(ktex_t));
  k->mutex = NULL;
#ifdef _WIN32
  k->mutex = CreateMutex(NULL, FALSE, "ktex");
  if(k->mutex == NULL)
  {
    free(k);
    return NULL;
  }
  return k;
#else
  if(pthread_mutex_init(k->mutex, NULL) != 0)
  {
    free(k);
    return NULL;
  }

  return k;
#endif
}

void ktex_destroy(ktex_t *k)
{
#ifdef _WIN32
  CloseHandle(k->mutex);
  free(k);
#else
  pthread_mutex_destroy(k->mutex);
  free(k);
#endif
}

void ktex_lock(ktex_t *k)
{
#ifdef _WIN32
  WaitForSingleObject( 
        k->mutex,    // handle to mutex
        INFINITE);  // no time-out interval
#else
  pthread_mutex_lock(k->mutex);
#endif
}

void ktex_unlock(ktex_t *k)
{
#ifdef _WIN32
  ReleaseMutex(k->mutex);
#else
  pthread_mutex_unlock(k->mutex);
#endif
}