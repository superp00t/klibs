#include "kqueue.h"
// Constructor
kq_elem *kq_elem_alloc()
{
  kq_elem *k = (kq_elem*)malloc(sizeof(kq_elem));
  memset(k, 0, sizeof(kq_t));
  k->next = NULL;
  k->ptr  = NULL;
  return k;
}

// Constructor
kq_t *kq_alloc()
{
  kq_t *k = (kq_t*)malloc(sizeof(kq_t));
  memset(k, 0, sizeof(kq_t));
  k->head = NULL;
  k->size = 0;
  return k;
}

// Get last linked element
kq_elem *kq_traverse_elem(kq_elem *k)
{
 if(k->next == NULL)
 {
  return k;
 }
 else {
  return kq_traverse_elem((kq_elem*)k->next);
 }
}

// Append element to queue
void kq_push(kq_t *k, void *ptr)
{
  kq_elem *ke;
  k->size++;
  if(k->head != NULL)
  {
    ke = kq_traverse_elem(k->head);
    kq_elem *kn = kq_elem_alloc();
    kn->ptr = ptr;
    ke->next = kn;
    return;
  }
  else
  {
    ke = kq_elem_alloc();
    ke->ptr = ptr;
    k->head = ke;
    return;
  }
}

void *kq_pop(kq_t *k)
{
  if(k->head == NULL)
  {
    return NULL;
  }
  kq_elem *h = k->head;
  void *ptr = h->ptr;
  kq_elem *n = (kq_elem*)h->next;
  free(h);
  k->head = n;
  k->size--;
  return ptr;
}

// Recursively delete queue elements
void kq_search_and_destroy(kq_elem *k)
{
  if(k->next == NULL)
  {
    free(k);
    return;
  }

  kq_elem *knxt = k->next;
  free(k);

  kq_search_and_destroy(knxt);
}

// Destructor
void kq_destroy(kq_t *k)
{
  if(k->head != NULL)
  {
    kq_search_and_destroy(k->head);
  }

  free(k);
}

// int main()
// {
//   kq_t *q = kq_alloc();
//   int x = 69;
//   int y = 420;
//   int z = 1337;

//   kq_push(q, (void*)&x);
//   kq_push(q, (void*)&y);
//   kq_push(q, (void*)&z);

//   int *a;
//   int *b;
//   int *c;

//   while(q->size > 0)
//   {
//     int *t = (int*)kq_pop(q);
//     printf("[%d] %d\n", q->size, *t);
//   }

//   kq_destroy(q);
// }
