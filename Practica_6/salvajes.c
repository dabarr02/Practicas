#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "defs.h"

#define NUMITER 3

share_cocina *ptr; //memoria compartida
sem_t *sem; //semaforo

int getServingsFromPot(void)
{
	ptr->nr_servings--;
	if(ptr->nr_servings==0){
		ptr->sav_wait=1;//los salvajes esperan porque no hay comida
	}

}

void eat(void)
{
	unsigned long id = (unsigned long) getpid();
	printf("Savage %lu eating\n", id);
	sleep(rand() % 5);
}

void savages(void)
{
	for (int i = 0; i < NUMITER; i++){
		sem_wait(sem);
		if(!ptr->sav_wait)
			getServingsFromPot();
		else{
			ptr->cook_wait=0;//Avisamos al cocinero
		}
		sem_post(sem);
		eat();
	}
}

int main(int argc, char *argv[])
{
	const char *name= "cocina"; //nombre de la memoria compartida
	const size_t size= sizeof(share_cocina); //tamaño que tiene
	const char *sem_name = "/sem_cocina"; // nombre del semáforo

    // Abrir el objeto de memoria compartida
    int shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
		printf("Error al abrir memoria compartida, ejecutar primero ./cocinero\n");
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Mapear el objeto de memoria compartida en el espacio de direcciones del proceso
   	ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

	//Abrimos el semaforo
    sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    savages();
    // Desmapear la memoria compartida
    if (munmap(ptr, size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // Cerrar el descriptor de archivo
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;

	return 0;
}
