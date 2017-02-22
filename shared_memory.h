#ifndef __SHARED_MEMORY__
#define __SHARED_MEMORY__

// Ouvre le segment de mémoire partagée, le projette dans l'espace 
//  mémoire de l'application et renvoie un pointeur sur le segment en
//  cas de succès, NULL en cas d'erreur
void *open_shared_memory(void);

#endif
