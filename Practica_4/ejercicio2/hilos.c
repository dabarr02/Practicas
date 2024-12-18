#include <stdlib.h>
#include <stdio.h>
#include<pthread.h>
#define num_threads 10
typedef struct { //estructura que contiene todos los paramtros que se le quiere pasar al hilo.
    int thread_num;
    char priority;
} thread_arg;

//Funcion que va a ejecutar el hilo cuando haga join()
void *thread_usuario(void *arg)
{
	//variable local para guardar los paramtros que nos pasan
	thread_arg* t_arg = (thread_arg*)arg; //hacemos casteo de tipos, default es void* y queremos que sea (thread_arg*)
	int id= t_arg->thread_num; //nos guardamos en varables locales la informacion
	char prio=t_arg->priority;
	free(t_arg); //liberamos memoria
	pthread_t thread_id=pthread_self(); //Averiguamos nuestro ID de hilo
	printf("Thread ID: %lu, Thread Number: %d, Priority: %c\n", thread_id, id, prio); //Imprimimos la info 
	pthread_exit(EXIT_SUCCESS); //salimos con exito 0
}

int main(int argc, char* argv[])
{
	
	pthread_t threads[num_threads]; // Vector de hilos, guardamos los identificadores de hilos creados, para luego usarlo en el join
	for (int i=0;i<num_threads;i++){ 
		thread_arg* argumento_hilo = (thread_arg*)malloc(sizeof(thread_arg));//reservamos memoria para los argumentos que vamos a pasar al hilo
		if(argumento_hilo==NULL){
			perror("Error asignar memoria");
			exit(EXIT_FAILURE);
		}
		//--Asignamos valores-----------
		argumento_hilo->thread_num=i;
		if(i%2==0)//asignamos prioridad
			argumento_hilo->priority='P';
		else
			argumento_hilo->priority='N';
		//--------------------------------
		//Creamos el hilo, guardando su identificador en threads[i], le decimos que va a ejecutar la funcion "thread_usuario" y le pasamos como argumento (void*) argumento_hilo
		if(pthread_create(&threads[i],NULL,thread_usuario,argumento_hilo)!=0){//creamos los hilos
			perror("Error crear hilo");
			free(argumento_hilo);
			exit(EXIT_FAILURE);
		}
		//repetimos para cada hilo que vamos a crear
		
	}
	for (int j = 0; j < num_threads; j++)//los hilos ejecutan su funcion
		{
			pthread_join(threads[j],NULL);
		}


	return 0;
}
