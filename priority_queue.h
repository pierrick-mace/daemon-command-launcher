#ifndef __PRIORITY_QUEUE__
#define __PRIORITY_QUEUE__

// Enlève une requète de la file d'attente et renvoie un pointeur sur 
//  variable allouée sur le tas contenant une copie de la requète afin
//  de pouvoir l'utiliser dans le thread approprié en cas de succès,
//  et NULL en cas d'erreur
requete *retrieve_from_queue(shared_mem *handle);

// Place une requete dans la file d'attente
//  Renvoie NULL en cas d'échec et 1 en cas de succès
void *send_cmd(shared_mem *handle, requete *req);

#endif
