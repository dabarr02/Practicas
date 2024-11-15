#include <stdlib.h>
#include <stdio.h>
#include<pthread.h>
#define num_threads 10
typedef struct {
    int thread_num;
    char priority;
} thread_arg;

void *thread_usuario(void *arg)
{
	thread_arg* t_arg = (thread_arg*)arg;
	int id= t_arg->thread_num;
	char prio=t_arg->priority;
	free(t_arg);
	pthread_t thread_id=pthread_self();
	printf("Thread ID: %lu, Thread Number: %d, Priority: %c\n", thread_id, id, prio);
	pthread_exit(0);
}

int main(int argc, char* argv[])
{
	
	pthread_t threads[num_threads];
	for (int i=0;i<num_threads;i++){ 
		thread_arg* argumento_hilo = (thread_arg*)malloc(sizeof(thread_arg));//reservamos memoria
		if(argumento_hilo==NULL){
			perror("Error asignar memoria");
			exit(EXIT_FAILURE);
		}
		argumento_hilo->thread_num=i;
		if(i%2==0)//asignamos prioridad
			argumento_hilo->priority='P';
		else
			argumento_hilo->priority='N';
		if(pthread_create(&threads[i],NULL,thread_usuario,argumento_hilo)!=0){//creamos los hilos
			perror("Error crear hilo");
			free(argumento_hilo);
			exit(EXIT_FAILURE);
		}
		
		
	}
	for (int j = 0; j < num_threads; j++)//los hilos ejecutan su funcion
		{
			pthread_join(threads[j],NULL);
		}


	return 0;
}
