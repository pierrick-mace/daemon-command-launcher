#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include "defines.h"
#include "priority_queue.h"
#include "threads.h"
#include "utils.h"
#include "shared_memory.h"

int main(int argc, char *argv[]) {
  
  pid_t pid;
  shared_mem *handle;
  int fifo_fd[2];
  char pipe_name[2][PIPE_NAME_MAX];
  pthread_t thread_input;
  pthread_t thread_output;
  client_thread_struct th_struct;
  sem_t thread_semaphore;
  
  if (argc < 2) {
    fprintf(stderr, "Usage: %s cmd [options]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  pid = getpid(); 
  
  // Ouverture du segment de mémoire partagée
  handle = (shared_mem *) open_shared_memory();
  if (handle == NULL) {
    exit(EXIT_FAILURE);    
  }
  
  // Initialisation du sémpahore, le sémpahore indique quand la commande
  //  à terminé de s'exécuter, ou que le client souhaite terminer 
  //  l'exécution du programme (avec CTRL + D)
  if (sem_init(&thread_semaphore, 1, 0) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }
  
  // Création de la requête  
  requete req;
  
  strncpy(req.cmd, argv[1], CMD_SIZE);
  req.pid = pid;
  if (getcwd(req.dir, DIR_LENGTH_MAX) == NULL) {
    perror("getcwd");
    exit(EXIT_FAILURE);
  }

  strncpy(req.args, "", CMD_SIZE);
  for (int i = 1; i < argc; i++) {
    strncat(req.args, argv[i], CMD_SIZE - (strlen(req.args) + 1));
    strncat(req.args, " ", CMD_SIZE - (strlen(req.args) + 1));
  }
  
  // Génération du nom des tubes
  generate_pipe_names(pipe_name, pid);

  // Création des tubes (mkfifo)
  if (create_pipes(pipe_name) == -1) {
    exit(EXIT_FAILURE);
  }
  
  // Envoi de la requête
  if (send_cmd(handle, &req) == NULL) {
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);
    exit(EXIT_FAILURE);    
  }

  // Ouverture des tubes
  if ((fifo_fd[0] = open(pipe_name[0], O_WRONLY)) == -1) {
    perror("open client 1");
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);
    exit(EXIT_FAILURE);
  }
  
  if ((fifo_fd[1] = open(pipe_name[1], O_RDONLY)) == -1) {
    perror("open client 2");
    close(fifo_fd[0]);
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);
    exit(EXIT_FAILURE);
  }
  
  // Initialisation de la structure contenant les données nécessaires
  //  aux threads
  th_struct.pipe_fd[0] = fifo_fd[0];
  th_struct.pipe_fd[1] = fifo_fd[1];
  strcpy(th_struct.pipe_name[0], pipe_name[0]);
  strcpy(th_struct.pipe_name[1], pipe_name[1]);
  th_struct.thread_semaphore = &thread_semaphore;
  
  // Création du thread qui lit sur l'entrée standard et écrit sur le tube
  int err;
  if ((err = pthread_create(&thread_input, NULL, client_read_routine, (void *) &th_struct)) != 0) {
    fprintf(stderr, "Error: pthread_create (%s)\n", strerror(err));
    close(fifo_fd[0]);
    close(fifo_fd[1]);
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);        
    exit(EXIT_FAILURE);
  }
  
  // Création du thread qui lit sur le tube et écrit sur la sortie standard
  if ((err = pthread_create(&thread_output, NULL, client_write_routine, (void *) &th_struct)) != 0) {
    perror("pthread_create");
    close(fifo_fd[0]);
    close(fifo_fd[1]);
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);        
    exit(EXIT_FAILURE);
  }
   
  // On attend qu'un des deux threads ait fini
  if (sem_wait(&thread_semaphore) == -1) {
    perror("sem_wait");
    close(fifo_fd[0]);
    close(fifo_fd[1]);
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);        
    exit(EXIT_FAILURE);    
  }
  
  // Libération des ressources des threads
  pthread_cancel(thread_output);
  pthread_join(thread_output, NULL);
  pthread_cancel(thread_input);
  pthread_join(thread_input, NULL);
    
  // Fermeture des descripteurs de fichier des tubes
  if (close(fifo_fd[0]) == -1) {
    perror("close");
    close(fifo_fd[1]);
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);
    exit(EXIT_FAILURE);
  }
  
  if (close(fifo_fd[1]) == -1) {
    perror("close");
    unlink(pipe_name[0]);
    unlink(pipe_name[1]);
    exit(EXIT_FAILURE);
  }
  
  // Suppression des tubes
  if (unlink(pipe_name[0]) == -1) {
    perror("unlink");
    unlink(pipe_name[1]);
    exit(EXIT_FAILURE);
  }
  
  if (unlink(pipe_name[1]) == -1) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}
