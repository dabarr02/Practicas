#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "defs.h"

#define NUMITER 3

share_cocina *ptr; //memoria compartida
sem_t *sem_cocinero; //semaforo
sem_t *sem_salvajes; //semaforo

int getServingsFromPot(void)
{
	ptr->nr_servings--;
    printf("[Salvaje] Comida consumida: %d\n",ptr->nr_servings);

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
		sem_wait(sem_salvajes);
        if(ptr->nr_servings==0){
            printf("Avisando Cocinero\n");
            sem_post(sem_cocinero);
            sem_wait(sem_salvajes);            
        }
		getServingsFromPot();
        printf("[Salvaje] Despertar salvajes\n");
        sem_post(sem_salvajes);
		eat();
	}
}

int main(int argc, char *argv[])
{
	const char *name= "cocina"; //nombre de la memoria compartida
	const size_t size= sizeof(share_cocina); //tamaño que tiene
	const char *sem_name_cook = "sem_cocinero"; // nombre del semáforo
    const char *sem_name_savg = "sem_salvajes";


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
    sem_cocinero = sem_open(sem_name_cook,0);
    if (sem_cocinero == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_salvajes = sem_open(sem_name_savg,0);
    if (sem_salvajes == SEM_FAILED) {
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
