#ifndef __defines_h__
#define __defines_h__

#include "sllist.h"

#define DIR_LENGTH_MAX 1024
#define CMD_SIZE 1024
#define SHM_NAME "/daemon_shm_25425451"
#define SHM_ARRAY_SIZE 10
#define SHM_SIZE sizeof(shared_mem)
#define LOG_NAME "daemon_log.txt"
#define LOG_BUFFER_SIZE 1024
#define PIPE_PREFIX "daemon_pipe_2623_"
#define PIPE_NAME_MAX 64
#define ARG_SIZE 2048
#define BUFFER_SIZE 4096

typedef struct requete {
  char dir[DIR_LENGTH_MAX];
  char cmd[CMD_SIZE];
  char args[ARG_SIZE];
  pid_t pid;
  int logfd;
  sllist *malloc_dispose;
} requete;

typedef struct shared_mem {
  sem_t empty;
  sem_t full;
  sem_t mutex;
  int head;
  int tail;
  requete buffer[SHM_ARRAY_SIZE];
} shared_mem;

typedef struct client_thread_struct {
  sem_t *thread_semaphore;
  pid_t main_pid;
  int pipe_fd[2];
  char pipe_name[2][PIPE_NAME_MAX];
} client_thread_struct;

#endif
