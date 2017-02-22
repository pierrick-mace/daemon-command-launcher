#ifndef SLLIST_H
#define SLLIST_H

typedef struct sllist sllist;

//  sllist_empty : crée une liste dynamique simplement chainée sur le type
//    void * initialement vide. Renvoie un pointeur sur l'objet qui gère cette
//    liste en cas de succès et NULL en cas d'échec
extern sllist *sllist_empty(void);

//  sllist_insert_tail : insère x en queue de la liste associée à sll. Renvoie x
//    en cas de succès, NULL en cas d'échec
extern void *sllist_insert_tail(sllist *sll, void *x);

//  sllist_apply : applique fun(x) à tous les éléments x de la liste associée à
//    sll, de la tête à la queue
extern void sllist_apply(sllist *sll, void (*fun)(void *));

//  sllist_apply2 : applique fun(x, funcontext(context, x)) à tous les éléments
//    x de la liste associée à sll, de la tête à la queue
extern void sllist_apply2(sllist *sll, void (*fun)(void *arg1, void *arg2),
    void *(*funcontext)(void *context, void *arg1), void *context);

//  sllist_dispose : libère les ressources allouées à *ptrsll ainsi qu'à la
//    liste associée. Donne à *ptrsll la valeur NULL
extern void sllist_dispose(sllist **ptrsll);

extern void sllist_sort(sllist *sl, int (*compar)(const void *, const void *));
#endif  // SLLIST_H
