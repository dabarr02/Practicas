#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "defs.h"
#include <semaphore.h>
#define M 10

int finish = 0;
share_cocina *ptr; //memoria compartida
sem_t *sem; //semaforo


void putServingsInPot(int servings)
{
	//Entramos a cocinar
	ptr->cook_wait=1 ;//nos toca esperar despues
	ptr->nr_servings+=servings; //recargamos los servings
	ptr->sav_wait=0;//los salvajes ya no esperan

}

void cook(void)
{
	while(!finish) {
		sem_wait(sem);//esperamos a que se libere el semaforo
		if(!ptr->cook_wait) //si el cocinero no tiene que esperar cook_wait = 0
			putServingsInPot(M);
		sem_post(sem);//ya hemos acabado avisamos al siguiente
	}
}

void handler(int signo)
{
	
	finish = 1;

}

int main(int argc, char *argv[])
{
	
//--Aqui manejamos la gestión de las señales para parar el programa.----------------------------------

	struct sigaction sa; //estructura de la accción 
    sa.sa_handler = handler; //Esta es la funcion que se ejecutara con sigalarm
    sa.sa_flags = 0;

	if (sigaction(SIGTERM, &sa, NULL) == -1) { //cambiamos la accion de SIGALARM
        perror("sigaction SIGTERM ");
        exit(EXIT_FAILURE);
    }
	if (sigaction(SIGINT, &sa, NULL) == -1) { //cambiamos la accion de SIGALARM
        perror("sigaction SIGINT ");
        exit(EXIT_FAILURE);
    }

	//-----------Fin Gestion de señales----------

	//----Creación de recursos compartidos-------
	const char *name= "cocina"; //nombre de la memoria compartida
	const size_t size= sizeof(share_cocina); //tamaño que le vamos a dar
	const char *sem_name = "/sem_cocina"; // nombre del semáforo

	int shm_fd = shm_open(name,O_CREAT | O_RDWR, 0666); //creamos la memoria compartida con nombre name, creat para crear, rdwr para leer y escribir, 0666 permisos
	if (shm_fd == -1) { //si falla mandar error
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

	// Establecer el tamaño del objeto de memoria compartida !!!!!!!!!! Sin esto no funcinona, por que no has reservado el espacio!!!!!!!!!!!!!
    if (ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

	ptr = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0); //Null para que elija el compilador la zona de momoria, tamaño, flags para que se pueda leer y escribir,
	//shared para que todos vean los cambios, el descriptor de fichero de la memoria compartida,0 el offset
	if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
	
	// Crear el semáforo
    sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

	ptr->cook_wait=0;
	ptr->nr_servings=0;
	ptr->sav_wait=1;
	
	cook();
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

    // Eliminar el objeto de memoria compartida
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }


	return 0;
}
