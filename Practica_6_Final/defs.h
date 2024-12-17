#ifndef DEFS_H
#define DEFS_H

#define MAX_CHARS_NIF  9
#define MAX_PASSWD_LINE  255

typedef struct {
	int servings; 
	int sav_waiting;
	int cook_waiting;
} share_cocina;

static const char *sem_name_cook = "/sem_cocinero"; // nombre del semáforo
static const char *sem_name_savg = "/sem_salvajes"; //nombre del semaforo
static const char *sem_name_mtx = "/mutex_salvajes"; // nombre del semáforo
static const char *name= "/cocina"; //nombre de la memoria compartida
	
#endif