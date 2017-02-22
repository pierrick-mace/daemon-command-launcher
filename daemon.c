#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include "defines.h"
#include "sllist.h"
#include "shared_memory.h"
#include "priority_queue.h"
#include "utils.h"
#include "threads.h"

static volatile sig_atomic_t quit;

void signal_handler(int signum) {
  if (signum == SIGUSR1) {
    quit = true;
  }
}

int main(void) {
  
  pid_t process_id = 0;
  shared_mem *handle;
  int log_fd;
  char buffer[LOG_BUFFER_SIZE];
  sllist *memory_disposal;
  
  quit = false;
  
  // Gestion du signal SIGUSR1 afin de terminer l'exécution du démon
  //  proprement
  signal(SIGUSR1, signal_handler);
  
  // Duplication du processus
  process_id = fork();
  if (process_id < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  
  // Le processus père se termine immédiatement afin que le processus
  //  fils se rattache au PID 1 (init)
  if (process_id > 0) {
    printf("daemon started with pid: %d\n", process_id);
    exit(EXIT_SUCCESS);
  }
  
  // Allocation d'un liste chainée pour libérer la mémoire allouée
  //  dynamiquement en cas d'erreur
  if ((memory_disposal = sllist_empty()) == NULL) {
    perror("sllist_empty");
    exit(EXIT_FAILURE);
  }
  
  // Ouverture du segment de mémoire partagée
  handle = (shared_mem *) open_shared_memory();
  if (handle == NULL) {
    sllist_dispose(&memory_disposal);
    exit(EXIT_FAILURE);    
  }
  
  // Initialisation du sémaphore mutex
  if (sem_init(&handle -> mutex, 1, 1) == -1) {
    perror("sem_init");
    sllist_dispose(&memory_disposal);
    exit(EXIT_FAILURE);
  }

  // Initialisation du sémaphore 'vide'
  if (sem_init(&handle -> empty, 1, SHM_ARRAY_SIZE) == -1) {
    perror("sem_init");
    sllist_dispose(&memory_disposal);
    exit(EXIT_FAILURE);
  }

  // Initialisation du sémaphore 'plein'
  if (sem_init(&handle -> full, 1, 0) == -1) {
    perror("sem_init");
    sllist_dispose(&memory_disposal);
    exit(EXIT_FAILURE);
  }

  // Initialisation des valeurs de tête et queue
  handle -> head = 0;
  handle -> tail = 0;
  
  // Ouverture d'un fichier log
  log_fd = open(LOG_NAME, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  if (log_fd == -1) {
    perror("open");
    sllist_dispose(&memory_disposal);
    exit(EXIT_FAILURE);
  }
  
  strcpy(buffer, "");
  strncpy(buffer, "Log file init.\n", LOG_BUFFER_SIZE);
  if (write(log_fd, buffer, LOG_BUFFER_SIZE) == -1) {
    perror("write");
    sllist_dispose(&memory_disposal);
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  // Nouvelle session
  if(setsid() == (pid_t) -1) {
    perror("setsid");
    sllist_dispose(&memory_disposal);
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  // Changement de répertoire pour la racine
  if (chdir("/") == -1) {
    perror("chdir");
    sllist_dispose(&memory_disposal);
    close(log_fd);
    exit(EXIT_FAILURE);
  }

  // Fermeture des descripteurs de fichiers de l'entrée/sortie standard
  if (close(STDIN_FILENO) == -1) {
    perror("close");
    sllist_dispose(&memory_disposal);
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  if (close(STDOUT_FILENO) == -1) {
    perror("close");
    sllist_dispose(&memory_disposal);
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  // Redirection de la sortie standard vers le fichier log
  if (dup2(log_fd, STDERR_FILENO) == -1) {
    perror("dup2");
    sllist_dispose(&memory_disposal);
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  while (!quit) {
   
    // Pointeur sur la requête à envoyer en argument au thread
    requete *req = retrieve_from_queue(handle);
    if (req == NULL) {
      // Si le sémaphore à été intérrompu par le signal SIGUSR1, il 
      //  renvoie -1 et la fonction renvoie donc NULL en conséquence
      //  d'où la nécessité de tester s'il s'agit d'une erreur
      //  classique ou d'une intérruption par signal.
      if (quit) {
        break;
      }
      
      perror("malloc");
      sllist_dispose(&memory_disposal);
      close(log_fd);
      exit(EXIT_FAILURE);
    }
    
    // Insertion du pointeur sur la mémoire allouée dynamiquement dans
    //  une liste chainée pour pouvoir facilement libérer la mémoire
    //  en cas d'erreur
    if (sllist_insert_tail(memory_disposal, (void *)req) == NULL) {
      perror("sllist_insert_tail");
      sllist_dispose(&memory_disposal);
      close(log_fd);
      exit(EXIT_FAILURE);
    }
    
    // Le thread à également besoin du pointeur sur la liste chaînée
    //  afin de pouvoir libérer la mémoire si une erreur se produit
    //  dans le thread
    req -> malloc_dispose = memory_disposal;
    req -> logfd = log_fd;
    pthread_t th;
    
    // Lancement du thread qui executera la commande contenue dans req
    int err;
    if ((err = pthread_create(&th, NULL, daemon_thread_routine, (void *) req)) != 0) {
        fprintf(stderr, "Error: pthread_create (%s)\n", strerror(err));
        close(log_fd);
        sllist_dispose(&memory_disposal);
        exit(EXIT_FAILURE);
    }
    
    // Le thread est mis en mode détaché, pour pouvoir libérer les 
    //  ressources automatiquement lors de la fin du thread
    if (pthread_detach(th) == -1) {
      perror("pthread_detach");
      close(log_fd);
      sllist_dispose(&memory_disposal);
      exit(EXIT_FAILURE);
    }

    strcpy(buffer, "");
    strncpy(buffer, "Launched thread.\n", LOG_BUFFER_SIZE);
    if (write(log_fd, buffer, LOG_BUFFER_SIZE) == -1) {
      perror("write");
      close(log_fd);
      sllist_dispose(&memory_disposal);
      exit(EXIT_FAILURE);
    }
    
    // Le démon fait une pause d'une seconde après chaque tour de boucle
    //  afin d'économiser les ressources
    sleep(1);
  }
  
  // Libération de la mémoire
  sllist_dispose(&memory_disposal);
  
  // Fermeture des sémaphores
  if (sem_destroy(&handle -> mutex) == -1) {
    perror("sem_destroy");
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  if (sem_destroy(&handle -> full) == -1) {
    perror("sem_destroy");
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  if (sem_destroy(&handle -> empty) == -1) {
    perror("sem_destroy");
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  // Fermeture du segment de mémoire partagée
  if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink");
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  strcpy(buffer, "");
  strncpy(buffer, "daemon exited successfully!\n", LOG_BUFFER_SIZE);
  if (write(log_fd, buffer, LOG_BUFFER_SIZE) == -1) {
    perror("write");
    close(log_fd);
    exit(EXIT_FAILURE);
  }
  
  // Fermeture du descripteur de fichier log
  close(log_fd);
  
  return (EXIT_SUCCESS);
}
