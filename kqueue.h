#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct {
  void    *ptr;
  void    *next;
} kq_elem;

typedef struct {
  kq_elem  *head;
  int      size;
} kq_t;

kq_t *kq_alloc();
kq_elem *kq_elem_alloc();
void kq_push(kq_t *k, void *ptr);
kq_elem *kq_traverse_elem(kq_elem *k);
void *kq_pop(kq_t *k);
void kq_search_and_destroy(kq_elem *k);
void kq_destroy(kq_t *k);