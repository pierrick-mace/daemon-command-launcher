#ifndef __THREADS_H__
#define __THREADS_H__

// Routine que les threads du démon exécutent
void *daemon_thread_routine(void *args);

// Routine que les threads qui lisent sur le tube et écrivent sur la 
//  sortie standard exécutent
void *client_write_routine(void *args);

// Routine que les threads qui lisent sur l'entrée standard et écrivent 
//  sur le tube exécutent
void *client_read_routine(void *args);

#endif
