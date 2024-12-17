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
#define M 4

int finish = 0;
share_cocina *shared; //memoria compartida
sem_t *cook_queue; //semaforo para el cocinero
sem_t *sav_queue; //semaforo para salvajes
sem_t *sem_mtx; //semaforo que es un cerrojo

void putServingsInPot(int servings)
{
	
	printf("[Cocinero] Preparando %d raciones...\n", servings);

    return;
}

void prepareServingsSafe(int servings){
    sem_wait(sem_mtx); //Adquirimos el "cerrojo" para operar en zona critica
    while(shared->servings>0){//bucle como en variables condidcionales, no salimos si aun hay comida
        // cond_wait
        shared->cook_waiting++; //hay cocinero esperando
        sem_post(sem_mtx); //liberamos cerrojo, no vamos a hacer nada si ya hay comida
        sem_wait(cook_queue); //Esperamos en la cola de cocinero hasta que nos llamen
        printf("[Cocinero] Cocinero despierto\n");
        if(finish){//solo servimos si no ha acabado la ejecucion
            printf("[Cocinero] Fin de ejecucion\n");
            return;
        }
        sem_wait(sem_mtx);//esperamos apara adquirir el cerrojo para modificar seccion critica
    }
    //Si llegamos aquí es que no hay comida y nos han despertado
    putServingsInPot(servings); //hacemos el print
    shared->servings=servings;//servimos la comida
    printf("[Cocinero] Comida actual: %d\n",shared->servings);
    printf("[Cocinero] Despertando salvajes\n");
    //Ahora despertamos a todos los salvajes que haya
    //cond_broadcast
    while(shared->sav_waiting>0){
        sem_post(sav_queue);//avisamos a un salvaje
        shared->sav_waiting--;//lo quitamos de la cola
    }
    sem_post(sem_mtx); //liberamos el cerrojo para que los salvajes puedan hacer cosas
}

void cook(void)
{
	while(!finish) {
        prepareServingsSafe(M); //Mientras no acabe la ejecucion intentamos preparar  comida
    }
    return;
}
void handler(int signo)
{
	
	finish = 1;
    sem_post(cook_queue); // liberar al cocinero si está esperando

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
	/* Los nombres de los semaforos y la memoria compartida estan en el defs.h */
	const size_t size= sizeof(share_cocina); //tamaño que le vamos a dar
	

    // Eliminar semáforos y memoria compartida existentes para evitar conflictos de ejecuciones anteriores.
    sem_unlink(sem_name_cook);  // Eliminamos cualquier semáforo cocinero anterior.
    sem_unlink(sem_name_savg);  // Eliminamos cualquier semáforo de los salvajes anterior.
    sem_unlink(sem_name_mtx); // Eliminamos cualquier semaforo de cerrojo anterior.
    shm_unlink(name);  // Eliminamos cualquier memoria compartida anterior.

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

	shared = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0); //Null para que elija el compilador la zona de momoria, tamaño, flags para que se pueda leer y escribir,
	//shared para que todos vean los cambios, el descriptor de fichero de la memoria compartida,0 el offset
	if (shared == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
	
	// Crear el semáforo cocinero
    cook_queue = sem_open(sem_name_cook, O_CREAT, 0644,0);
    if (cook_queue == SEM_FAILED) {
        perror("sem_open");
        sem_unlink(sem_name_cook);  // Eliminamos cualquier semáforo cocinero anterior.
        shm_unlink(name);  // Eliminamos cualquier memoria compartida anterior.
        exit(EXIT_FAILURE);
    }
    // crear semaforo salvajes
    sav_queue = sem_open(sem_name_savg, O_CREAT, 0644, 0);
    if (sav_queue == SEM_FAILED) {
        perror("sem_open");
        sem_unlink(sem_name_cook);  // Eliminamos cualquier semáforo cocinero anterior.
        sem_unlink(sem_name_savg);  // Eliminamos cualquier semáforo de los salvajes anterior.
        shm_unlink(name);  // Eliminamos cualquier memoria compartida anterior.
        exit(EXIT_FAILURE);
    }
    sem_mtx = sem_open(sem_name_mtx, O_CREAT, 0644, 1);// crear semaforo salvajes
    if (sem_mtx == SEM_FAILED) {
        perror("sem_open");
        sem_unlink(sem_name_cook);  // Eliminamos cualquier semáforo cocinero anterior.
        sem_unlink(sem_name_savg);  // Eliminamos cualquier semáforo de los salvajes anterior.
        sem_unlink(sem_name_mtx); // Eliminamos cualquier semaforo de cerrojo anterior.
        shm_unlink(name);  // Eliminamos cualquier memoria compartida anterior.
        exit(EXIT_FAILURE);
    }

	shared->cook_waiting=0;
	shared->servings=0;
	shared->sav_waiting=0;
	
	cook();
	

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
    //Cerramos y desvinculamos cerrojos
    // Cerrar los semáforos
    if (sem_close(cook_queue) == -1) {
        perror("sem_close sem_cocinero");
        exit(EXIT_FAILURE);
    }

    if (sem_close(sav_queue) == -1) {
        perror("sem_close sem_salvajes");
        exit(EXIT_FAILURE);
    }
     if (sem_close(sem_mtx) == -1) {
        perror("sem_close sav_queue");
        exit(EXIT_FAILURE);
    }
    // Eliminar los semáforos
    if (sem_unlink(sem_name_cook) == -1) {
        perror("sem_unlink sem_cocinero");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(sem_name_savg) == -1) {
        perror("sem_unlink sav_queue");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(sem_name_mtx) == -1) {
        perror("sem_unlink sem_mtx");
        exit(EXIT_FAILURE);
    }
    
     // Desmapear la memoria compartida
    if (munmap(shared, size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }


	return 0;
}
