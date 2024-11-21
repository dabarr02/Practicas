#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <pthread.h>
#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")
#define NUM_THREADS 100
typedef struct {
    int thread_num;
    int priority;
} thread_arg;

pthread_cond_t entrada,entrada_vip;
pthread_mutex_t cerrojo;
int nr_vips=0;//vips que estan esperando
int aforo=0;

void enter_normal_client(int id)
{
	pthread_mutex_lock(&cerrojo);
	//aqui esperan para ver si entran
	while(aforo>=CAPACITY||nr_vips>0){//si esta lleno o aun quedan vips esperando, no entras
		pthread_cond_wait(&entrada,&cerrojo);
	}
	//reducimos aforo
	aforo++;
	
	pthread_mutex_unlock(&cerrojo);

}

void enter_vip_client(int id)
{
	//aqui esperan para ver si entran
	pthread_mutex_lock(&cerrojo);
	nr_vips++; //hay un vip esperando
	while(aforo>=CAPACITY){
		pthread_cond_wait(&entrada_vip,&cerrojo);
	}
	//si podemos entrar, aumentamos aforo y reducimos vips
	nr_vips--;
	aforo++;
	
	pthread_mutex_unlock(&cerrojo); //liberamos

}

void dance(int id, int isvip)
{
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip)
{
	printf("Client %2d (%s) exits disco\n", id, VIPSTR(isvip));
	pthread_mutex_lock(&cerrojo);
		aforo--;//salimos de la sala, hay un hueco
		if(nr_vips>0){//si hay vips despertamos al siguiente
			pthread_cond_signal(&entrada_vip);
		}else{//si no hay vips, entra el siguiente normal
			pthread_cond_signal(&entrada);
		}
	pthread_mutex_unlock(&cerrojo);
}

void *client(void *arg)
{
	thread_arg* t_arg = (thread_arg*)arg;
	int id= t_arg->thread_num;
	int prio=t_arg->priority;
	free(t_arg);
	if(prio==1){// es vip
		printf("Client %2d (%s) ESPERA disco\n", id, VIPSTR(prio));
		enter_vip_client(id);
		printf("Client %2d (%s) ENTRA en la disco\n", id, VIPSTR(prio));
	}else{
		printf("Client %2d (%s) ESPERA disco\n", id, VIPSTR(prio));
		enter_normal_client(id);
		printf("Client %2d (%s) ENTRA en la disco\n", id, VIPSTR(prio));
	}
	dance(id,prio);
	disco_exit(id,prio);
	pthread_exit(0);

}

int main(int argc, char *argv[])
{
	int n;
	int p;
	FILE* file= fopen("ejemplo.txt","r+");
	pthread_t threads[NUM_THREADS];
	if (pthread_mutex_init(&cerrojo, NULL) != 0) {//inicializamos cerrojo
		printf("Error al inicializar el mutex\n");
		return -1;
	}
	if (pthread_cond_init(&entrada, NULL) != 0) { //inicializamos var cond
    	printf("Error al inicializar la variable de condición\n");
    	return -1;
	}
	if (pthread_cond_init(&entrada_vip, NULL) != 0) { //inicializamos var cond
    	printf("Error al inicializar la variable de condición\n");
    	return -1;
	}
	if(fscanf(file,"%d\n",&n)!=1){
		perror("Error scan numero de clentes");
	}
	for(int i=0;i<n;i++){
		thread_arg* argumento_hilo = (thread_arg*)malloc(sizeof(thread_arg));//reservamos memoria
		if(argumento_hilo==NULL){
			perror("Error asignar memoria");
			exit(EXIT_FAILURE);
		}
		if(fscanf(file,"%d\n",&p)!=1){ //leemos prio
			perror("Error scan numero de clentes");
		}
		//rellenamos los argumentos del hilo
		argumento_hilo->priority=p;
		argumento_hilo->thread_num=i;
		//Creamos el hilo
		if(pthread_create(&threads[i],NULL,client,argumento_hilo)!=0){
			perror("Error crear hilo");
			free(argumento_hilo);
			exit(EXIT_FAILURE);
		}
	}
	fclose(file);
	
	
	for (int j=0;j<n;j++){
		pthread_join(threads[j],NULL);
	}
	pthread_mutex_destroy(&cerrojo);
    pthread_cond_destroy(&entrada);
    pthread_cond_destroy(&entrada_vip);
	return 0;
}
