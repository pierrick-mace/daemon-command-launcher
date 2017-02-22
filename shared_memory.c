#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include "defines.h"

void *open_shared_memory(void) {
  // Ouverture du segment de mémoire partagée
  int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
  if (shm_fd == -1) {
    perror("shm_open");
    return NULL;
  }
    
  // Définition de la taille du segment de mémoire partagée
  if (ftruncate(shm_fd, SHM_SIZE) == -1) {
    perror("ftruncate");
    return NULL;
  }
  
  // Projection du segment de mémoire partagée sur l'espace d'adressage
  //  virtuel
  void *handle = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (handle == MAP_FAILED) {
    perror("mmap");
    return NULL;
  }  
  
  return handle;
}
