#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include "defines.h"
#include "priority_queue.h"

// Enlève une requète de la file d'attente et renvoie un pointeur sur 
//  variable allouée sur le tas contenant une copie de la requète afin
//  de pouvoir l'utiliser dans le thread approprié en cas de succès,
//  et NULL en cas d'erreur
requete *retrieve_from_queue(shared_mem *handle) {
  
  if (sem_wait(&handle -> full) == -1) {
    perror("sem_wait");
    return NULL;
  }
    
  if (sem_wait(&handle -> mutex) == -1) {
    perror("sem_wait");
    return NULL;
  }
  
  requete *req = malloc(sizeof(requete));
  if (req == NULL) {
    return NULL;
  }
  
  *req = handle -> buffer[handle -> tail];

  handle -> tail = (handle -> tail + 1) % SHM_ARRAY_SIZE;

  if (sem_post(&handle -> mutex) == -1) {
    perror("sem_post");
    return NULL;
  }
  
  if (sem_post(&handle -> empty) == -1) {
    perror("sem_post");
    return NULL;
  }
  
  return req;
}

// Place une requete dans la file d'attente
//  Renvoie NULL en cas d'échec et 1 en cas de succès
void *send_cmd(shared_mem *handle, requete *req) {
  if (sem_wait(&handle -> empty) == -1) {
    perror("sem_wait");
    return NULL;
  }
  
  if (sem_wait(&handle -> mutex) == -1) {
    perror("sem_wait");
    return NULL;
  }

  handle -> buffer[handle -> head] = *req;
  
  handle -> head = (handle -> head + 1) % SHM_ARRAY_SIZE;

  if (sem_post(&handle -> mutex) == -1) {
    perror("sem_post");
    return NULL;
  }
  
  if (sem_post(&handle -> full) == -1) {
    perror("sem_post");
    return NULL;
  }
  
  return (void *)1;
}
