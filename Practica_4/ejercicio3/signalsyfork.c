#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>


/*programa que temporiza la ejecución de un proceso hijo */
pid_t child_pid; //como handle_sigalarm solo puede recibir un argumento (signal number) el pid es global

void handle_sigalrm(int sig) { //Funcion que se ejecuta cuando se recibe una señal SIGALARM
    if (child_pid > 0) {
        printf("Parent: Sending SIGKILL to child process (PID: %d)\n", child_pid);
        kill(child_pid, SIGKILL); //Matamos al proceso con PID = child_pid
    }
}
void handle_sigINIT(int sig) { //Funcion que se ejecuta cuando recibimos un Ctrl + C
    printf("\nEstoy ignorando el Ctrl+C\n");
}
int main(int argc, char **argv)
{
	 if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //------------ Manejo de SEÑALES---------------
    //Estructura donde guardamos los parametros que el pasamos para la accion al recibir la señal
	struct sigaction sa; //estructura de la accción 
    sa.sa_handler = handle_sigalrm; //Esta es la funcion que se ejecutara con sigalarm
    sa.sa_flags = 0;

	
    //Aqui decimos que: Si recibo un SIGALARM, usa los paramtros que hay en sa (donde hemos puesto que la funcion que se encarga de manejar esa señal es handler_sigalrm)
	if (sigaction(SIGALRM, &sa, NULL) == -1) { //cambiamos la accion de SIGALARM
        perror("sigaction ALARM");
        exit(EXIT_FAILURE);
    }
    //Lo mismo que lo anterior pero para la señal SIGINIT
	struct sigaction saInit;
    saInit.sa_handler = handle_sigINIT; //Esta es la funcion que se ejecutara con sigalarm
    saInit.sa_flags = 0;

	if (sigaction(SIGINT, &saInit, NULL) == -1) { //cambiamos la accion de SIGINIT
        perror("sigaction INIT");
        exit(EXIT_FAILURE);
    }
    //---------------- Fin manejo de SEÑALES
	child_pid = fork(); //creamos un hijo
	if (child_pid==-1){
		perror("Error fork");
		exit(EXIT_FAILURE);
	}
	if(child_pid==0){
        //---------------proceso hijo-----------------
        // El hijo ignora sigINIT tambien, si lo recibe antes de ejecutar execvp--------
		struct sigaction sa_child;
        sa_child.sa_handler = handle_sigINIT;
        sa_child.sa_flags = 0;
        
        if (sigaction(SIGINT, &sa_child, NULL) == -1) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
		if(execvp(argv[1], &argv[1])==-1){ // ejecuta el comando pasado como argumento con los argumentos necesarios pasados de forma independiente
			perror("execvp"); // Si execvp falla
			exit(EXIT_FAILURE);
		}
        //---------------------------------------------
	}else{
        //--------proceso padre------------------------
		alarm(5);//planeamos alarma en 5s
		int status;
		pid_t pid;
		//seguimos haciendo wait hasta que muera el hijo, ya que la SIGNAL detiene el wait y retorna -1 
		//con un errno= EINTR (error por interrupcion) y tenemos que seguir hasta que el hijo termine
        while ((pid = wait(&status)) == -1 && errno == EINTR) { 
            continue;
        }
		//si ha fallado por otro motivo manejamos el error
		if (pid == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
		if (WIFEXITED(status)) {
            printf("Child process (PID: %d) termino correctamente, con status= %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process (PID: %d) terminado por señal %d\n", pid, WTERMSIG(status));
        }
	}

	return 0;
}
