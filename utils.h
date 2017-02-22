#ifndef __UTILS_H__
#define __UTILS_H__


// Place les arguments dans un tableau de pointeur de chaine de 
//  caractères et renvoie un pointeur sur ce tableau
//  Renvoie un pointeur sur le tableau en cas de succès, NULL en cas
//  d'échec
char **parse_args(char *args, size_t len, sllist *memory_disposal_list);

// Compte et renvoie le nombre d'arguments contenu dans la chaine args
size_t get_args_number(char *args);

// Libération de la mémoire
void clean_memory(requete *args, char **args_array, size_t len);

// Génération du nom des tubes en fonction du PID
void generate_pipe_names(char name[2][PIPE_NAME_MAX], pid_t pid);

// Créé les tubes nommés en fonction de pipe_name
int create_pipes(char pipe_name[2][PIPE_NAME_MAX]);

#endif
