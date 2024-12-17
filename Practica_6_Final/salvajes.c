#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "defs.h"

#define NUMITER 3

share_cocina *shared; //memoria compartida
sem_t *cook_queue; //semaforo para el cocinero
sem_t *sav_queue; //semaforo para salvajes
sem_t *sem_mtx; //semaforo que es un cerrojo

int getServingsFromPot(void)
{   
    unsigned long id = (unsigned long) getpid();
    printf("[Salvage %lu] Voy a recibir comida, comida actual: %d\n",id,shared->servings);
    

}

void getServingSafe(){

    unsigned long id = (unsigned long) getpid();
    printf("[Salvage %lu] Esperando en 1er MTX\n", id);
    sem_wait(sem_mtx); //Adquirimos semaforo para entrar en la seccion critica
    while(shared->servings==0){//Si no hay comida no salimos de bucle
        //Como no hay comida, despertamos al cocinero
        if(shared->cook_waiting>0){ //Si hay cocinero esperando
            printf("[Salvage %lu] Despertando cocinero\n", id);
            sem_post(cook_queue); //Despertamos cocinero
            shared->cook_waiting--; //Quitamos un cocinero de la cola
        }
        //Ahora no bloqueamos y esperamos a que haya comida
        shared->sav_waiting++; //Hay un salvaje mas en la cola
        sem_post(sem_mtx);//Liberamos cerrojo
        printf("[Salvage %lu] Esperando en cola salvajes\n", id);
        sem_wait(sav_queue);//Esperamos en semaforo de salvajes
        sem_wait(sem_mtx); //Si nos despiertan esperamos a tener el cerrojo para entrar en seccion critica otra vez
        printf("[Salvage %lu] Saliendo de 2 MTX\n", id);
    }
    //Si llegamos aqui hay comida y podemos editar seccion critica
    getServingsFromPot();
    shared->servings--; //Cogemos la comida
    printf("[Salvaje %lu] Comida consumida,quedan : %d\n",id,shared->servings);
    sem_post(sem_mtx); //Liberamos cerrojo para que entren los demas salvajes, que estan despiertos (El cocinero despierta a todos)
    // O deja pasar a los que esten el primer wait del mtx antes del while
}

void eat(void)
{
	unsigned long id = (unsigned long) getpid();
	printf("[Savage %lu] Eating\n", id);
	sleep(rand() % 5);
}

void savages(void)
{
   
	for (int i = 0; i < NUMITER; i++) {
        getServingSafe();
        eat();
    }
}

int main(int argc, char *argv[])
{
	
	const size_t size= sizeof(share_cocina); //tamaño que tiene
	

    // Abrir el objeto de memoria compartida
    int shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
		printf("Error al abrir memoria compartida, ejecutar primero ./cocinero\n");
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Mapear el objeto de memoria compartida en el espacio de direcciones del proceso
   	shared = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
	//Abrimos el semaforo
    cook_queue = sem_open(sem_name_cook,0);
    if (cook_queue == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sav_queue = sem_open(sem_name_savg,0);
    if (sav_queue == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_mtx = sem_open(sem_name_mtx,0);
    if(sem_mtx==SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    savages();
    // Desmapear la memoria compartida
    if (munmap(shared, size) == -1) {
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
