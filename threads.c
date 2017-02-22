#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defines.h"
#include "utils.h"

void *daemon_thread_routine(void *arg) {
  
  requete *args = (requete *) arg;
  char buffer[LOG_BUFFER_SIZE];
  char pipe_name[2][PIPE_NAME_MAX];
  int fifo_fd[2];
  int error_log_fd;
  size_t arg_count = 0;
  char **args_array;
  char error_log_name[32];
  
  if (chdir(args -> dir) == -1) {
    strcpy(buffer, "");
    sprintf(buffer, "Error: chdir (%s) thread\n", args -> dir);
    write(args -> logfd, buffer, LOG_BUFFER_SIZE);
    clean_memory(args, NULL, 0);
    exit(EXIT_FAILURE);
  }
  
  generate_pipe_names(pipe_name, args -> pid);

  sprintf(error_log_name, "error_log_%d", (int) args -> pid);
  
  if ((error_log_fd = open(error_log_name, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
    perror("open error log");
    clean_memory(args, NULL, 0);
    exit(EXIT_FAILURE);
  }
  
  args -> logfd = error_log_fd;
  
  if (dup2(error_log_fd, STDERR_FILENO) == -1) {
    perror("dup2");
    clean_memory(args, NULL, 0);
    exit(EXIT_FAILURE);
  }
  
  arg_count = get_args_number(args -> args);
  args_array = parse_args(args -> args, arg_count, args -> malloc_dispose);

  if (args_array == NULL) {
    perror("malloc");
    clean_memory(args, args_array, arg_count);
    exit(EXIT_FAILURE);
  }
  
  switch (fork()) {
    case -1:
      perror("fork");
      clean_memory(args, args_array, arg_count);
      exit(EXIT_FAILURE);

    case 0:
    
      if ((fifo_fd[0] = open(pipe_name[0], O_RDONLY)) == -1) {
        perror("open fifo[0]");
        clean_memory(args, args_array, arg_count);
        exit(EXIT_FAILURE);
      }
              
      if ((fifo_fd[1] = open(pipe_name[1], O_WRONLY)) == -1) {
        perror("open fifo[1]");
        clean_memory(args, args_array, arg_count);
        exit(EXIT_FAILURE);
      }
           
      if (dup2(fifo_fd[0], STDIN_FILENO) == -1) {
        perror("dup2");
        clean_memory(args, args_array, arg_count);
        exit(EXIT_FAILURE);
      }
      
      if (dup2(fifo_fd[1], STDOUT_FILENO) == -1) {
        perror("dup2");
        clean_memory(args, args_array, arg_count);
        exit(EXIT_FAILURE);
      }
            
      if (dup2(fifo_fd[1], STDERR_FILENO) == -1) {
        perror("dup2");
        clean_memory(args, args_array, arg_count);
        exit(EXIT_FAILURE);
      }
      
      execvp(args -> cmd, args_array);
      perror("execvp");
      printf("Error: wrong command name.\n");
    
      clean_memory(args, args_array, arg_count);
      exit(EXIT_FAILURE);
    
    default:
      if (wait(NULL) == -1) {
        perror("wait");
        clean_memory(args, args_array, arg_count);
        exit(EXIT_FAILURE);
      }
  }
  
  clean_memory(args, args_array, arg_count);
  
  unlink(error_log_name);

  return NULL; 
}

void *client_read_routine(void *arg) {
  client_thread_struct *th_struct = (client_thread_struct *) arg;
  
  char buffer[BUFFER_SIZE];
  ssize_t n;
  
  while ((n = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
    if (write(th_struct -> pipe_fd[0], buffer, (size_t) n) == -1) {
      perror("write input");
      return NULL;
    }
  }

  if (n == -1) {
    perror("read");
    return NULL;
  }

  if (sem_post(th_struct -> thread_semaphore) == -1) {
    perror("sem_post");
    return NULL;
  }  
  
  return NULL;
}

void *client_write_routine(void *arg) {
  client_thread_struct *th_struct = (client_thread_struct *) arg;
  
  char buffer[BUFFER_SIZE];
  ssize_t n;
  
  while ((n = read(th_struct -> pipe_fd[1], buffer, BUFFER_SIZE)) > 0) {
    if (write(STDOUT_FILENO, buffer, (size_t) n) == -1) {
      perror("write output");
      return NULL;
    }    
  }

  if (n == -1) {
    perror("read");
    return NULL;
  }
  
  if (sem_post(th_struct -> thread_semaphore) == -1) {
    perror("sem_post");
    return NULL;
  }
  
  return NULL;
}
