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

void handle_sigalrm(int sig) {
    if (child_pid > 0) {
        printf("Parent: Sending SIGKILL to child process (PID: %d)\n", child_pid);
        kill(child_pid, SIGKILL);
    }
}
void handle_sigINIT(int sig) {
    printf("\nEstoy ignorando el Ctrl+C\n");
}
int main(int argc, char **argv)
{
	 if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	struct sigaction sa; //estructura de la accción 
    sa.sa_handler = handle_sigalrm; //Esta es la funcion que se ejecutara con sigalarm
    sa.sa_flags = 0;

	

	if (sigaction(SIGALRM, &sa, NULL) == -1) { //cambiamos la accion de SIGALARM
        perror("sigaction ALARM");
        exit(EXIT_FAILURE);
    }

	struct sigaction saInit;
    saInit.sa_handler = handle_sigINIT; //Esta es la funcion que se ejecutara con sigalarm
    saInit.sa_flags = 0;

	if (sigaction(SIGINT, &saInit, NULL) == -1) { //cambiamos la accion de SIGINIT
        perror("sigaction INIT");
        exit(EXIT_FAILURE);
    }

	child_pid = fork();
	if (child_pid==-1){
		perror("Error fork");
		exit(EXIT_FAILURE);
	}
	if(child_pid==0){//proceso hijo
		struct sigaction sa_child;
        sa_child.sa_handler = handle_sigINIT;
        sa_child.sa_flags = 0;
        
        if (sigaction(SIGINT, &sa_child, NULL) == -1) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
		if(execvp(argv[1], &argv[1])==-1){
			perror("execvp"); // Si execvp falla
			exit(EXIT_FAILURE);
		}
		exit(0);
	}else{//proceso padre
		alarm(5);//planeamos alarma en 5s
		int status;
		pid_t pid;
		//seguimos haciendo wait hasta que muera el hijo, ya que la SIGNAL para el wait y retorna -1 
		//con un errno= EINTR (error por interrupcion)
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
