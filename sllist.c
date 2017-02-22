#include <stdlib.h>
#include "sllist.h"

typedef struct csllist csllist;

struct csllist {
  void *value;
  csllist *next;
};

struct sllist {
  csllist *head;
  csllist *tail;
};

sllist *sllist_empty(void) {
  sllist *sll = malloc(sizeof *sll);
  if (sll == NULL) {
    return NULL;
  }
  sll -> head = NULL;
  return sll;
}

void *sllist_insert_tail(sllist *sll, void *x) {
  if (x == NULL) {
    return NULL;
  }
  csllist *p = malloc(sizeof *p);
  if (p == NULL) {
    return NULL;
  }
  p -> value = x;
  p -> next = NULL;
  if (sll -> head == NULL) {
    sll -> head = p;
  } else {
    sll -> tail -> next = p;
  }
  sll -> tail = p;
  return x;
}

void sllist_apply(sllist *sll, void (*fun)(void *)) {
  csllist *p = sll -> head;
  while (p != NULL) {
    fun(p -> value);
    p = p -> next;
  }
}

void sllist_apply2(sllist *sll, void (*fun)(void *arg1, void *arg2),
    void *(*funcontext)(void *context, void *arg1), void *context) {
  csllist *p = sll -> head;
  while (p != NULL) {
    fun(p -> value, funcontext(context, p -> value));
    p = p -> next;
  }
}

void sllist_dispose(sllist **ptrsll) {
  csllist *p = (*ptrsll) -> head;
  while (p != NULL) {
    csllist *t = p;
    p = p -> next;
    free(t);
  }
  free(*ptrsll);
  *ptrsll = NULL;
}

void sllist_sort(sllist *sl, int (*compar)(const void *, const void *))
{
  if(sl->head != NULL) 
  {
    sllist *l1 = sllist_empty();
  
    csllist *p = sl -> head;
    sl -> head = p -> next;
    p->next = l1 -> head;
    l1 -> head = p;
    
    if(sl->head == NULL) 
    {
      p = l1 -> head;
      l1 -> head = p -> next;
      p->next = sl -> head;
      sl -> head = p;
    } 
    
    else 
    {
      sllist *l2 = sllist_empty();
      while(sl->head != NULL) 
      {       
        p = sl->head;
        sl->head = p->next;
        p->next = l2->head;
        l2->head = p;
              
        
        if(sl->head != NULL) 
        {
          p = sl->head;
          sl->head = p->next;
          p->next = l1->head;
          l1->head = p;
        }
      }
  
      sllist_sort(l1, compar);
      sllist_sort(l2, compar);
      sllist *tmp = sllist_empty();
      
      while (l1->head != NULL || l2->head != NULL) 
      {
        if(l2->head == NULL || (l1->head != NULL
        && compar(l1 -> head -> value, l2 -> head -> value) < 0))
        {
          p = l1->head;
          l1->head = p->next;
          p->next = tmp->head;
          tmp->head = p;
        } 
        
        else 
        {
          p = l2->head;
          l2->head = p->next;
          p->next = tmp->head;
          tmp->head = p;
        }
      }
        
      while (tmp->head != NULL) 
      {
        p = tmp->head;
        tmp->head = p->next;
        p->next = sl->head;
        sl->head = p;
      }
        
      sllist_dispose(&l2);
      sllist_dispose(&tmp);    
    }
    sllist_dispose(&l1);
  }
}
