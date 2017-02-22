#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "defines.h"

// Compte et renvoie le nombre d'arguments contenu dans la chaine args
size_t get_args_number(char *args) {
  size_t arg_count = 0;
  
  size_t len = strlen(args) + 1;
  bool space_before = true;

  for (size_t i = 0; i < len; i++) {
    if (args[i] != ' ' && space_before) {
      arg_count++;
    }
    
    space_before = args[i] == ' ';
  }
  
  return arg_count;  
}

// Place les arguments dans un tableau de pointeur de chaine de 
//  caractères et renvoie un pointeur sur ce tableau
//  Renvoie un pointeur sur le tableau en cas de succès, NULL en cas
//  d'échec
char **parse_args(char *args, size_t len, sllist *memory_disposal_list) {
  
  char **args_array = malloc(len * sizeof(char *));
  if (args_array == NULL) {
    perror("malloc");
    return NULL;
  }
  
  size_t i = 0;
  char *token = strtok(args, " ");

  while (token != NULL) {
    char *tmp = malloc(sizeof(char) * (strlen(token) + 1));
    if (tmp == NULL) {
      for (size_t i = 0; i < len; i++) {
        if (args_array[i] != NULL) {
          free(args_array[i]);
        }
      }
      
      free(args_array);
      perror("malloc");
      return NULL;
    }
    
    if (sllist_insert_tail(memory_disposal_list, (void *)tmp) == NULL) {
      for (size_t i = 0; i < len; i++) {
        if (args_array[i] != NULL) {
          free(args_array[i]);
        }
      }
      
      free(args_array);
      perror("sllist_insert_tail");
      return NULL;
    }
    
    strcpy(tmp, token);
    args_array[i] = tmp;
    token = strtok(NULL, " ");
    i++;
  }
  
  if (sllist_insert_tail(memory_disposal_list, (void *)args_array) == NULL) {
    for (size_t i = 0; i < len; i++) {
      if (args_array[i] != NULL) {
        free(args_array[i]);
      }
    }
    
    free(args_array);
    perror("sllist_insert_tail");
    return NULL;    
  }

  args_array[i] = NULL;  
  
  return args_array;
}

// Libération de la mémoire
void clean_memory(requete *args, char **args_array, size_t len) {
  close(args -> logfd);
  
  if (args_array != NULL) {
    for (size_t i = 0; i < len - 1; i++) {
      free(args_array[i]);
    }
  
    free(args_array);
  }
  
  free(args);
}

// Génération du nom des tubes en fonction du PID
void generate_pipe_names(char name[2][PIPE_NAME_MAX], pid_t pid) {
  sprintf(name[0], PIPE_PREFIX"%d_0", (int) pid);
  sprintf(name[1], PIPE_PREFIX"%d_1", (int) pid);
}

// Créé les tubes nommés en fonction de pipe_name
int create_pipes(char pipe_name[2][PIPE_NAME_MAX]) {
  
  if (mkfifo(pipe_name[0], S_IRUSR | S_IWUSR) == -1) {
    perror("mkfifo");
    return -1;
  }
  
  if (mkfifo(pipe_name[1], S_IRUSR | S_IWUSR) == -1) {
    perror("mkfifo");
    return -1;
  }
  
  return 0;
}
