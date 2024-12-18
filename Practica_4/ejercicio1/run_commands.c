#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include "defs.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>



//Función para ejecutar el comando que se pasa por el parametro argv (argv[0] es el comando y el resto los parametros de este)
pid_t launch_command(char** argv,int c){
    //Creamos proceso hijo
    pid_t PID= fork(); 
    if(PID==0){
//-----------Inicio Hijo-------------        
        if(c!=-1){ //c = -1 cuando llamamos desde la opcion -x, en este caso no queremos hacer print
            printf("@@ Running command #%d:", c); //imprime el comando que se ejecuta y todos sus argumentos
            for (int i = 0; argv[i] != NULL; i++) {
                printf(" %s", argv[i]);
            }
            printf("\n");
        }
        if( execvp(argv[0],argv)==-1){ // Ejecuta el comando almacenado en argv[] si tiene exito el hijo pasa de esta linea, si falla exit_failure (1)
            perror("execv");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE); 
//----------Fin hijo------------
    }else if(PID<0){
        perror("fork");
        exit(EXIT_FAILURE);
    }
// Devolvemos el PID del hijo que hemos creado (el padre adopta el pid del hijo en el fork())
   return PID;
   
}


// Funcion que parsea un comando escrito en una linea "ejemploComando <sus argumento>" y devuelve un vector de strings (char**) en el que cada elemento es un argumento (argv[0] es el comando) 
//argc es el numero de elementos de argv, incluido el nombre del comando (numero de argumentos es argc - 1)
 char **parse_command(const char *cmd, int* argc) {
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));
    
    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc)=arg_count; // Return argc
    return argv;
}
/*----------Funcion para Opcion -s ------------------------
    Ejecuta todos los comandos escritos en cada linea del fichero de texto con el nombre pasado en el argumento "file_name"
*/
void ejecuta_fichero(char* file_name){
    /*
        Abrimos el archivo <file_name> sus descriptor ahora es "file"
    */
    FILE* file;
    file = fopen(file_name,"r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE); 
    }
    // Variable para guardar la linea leida (el maximo es 256 caracteres)
    char comando[MAX_CMD_LINE];
    char **cmd_argv; //variable para guardar el cada argumento del comando leido, despues del parse (es un vector de strings)
    pid_t pidcmd; //Guardamos el PID del hijo que ejecutará el comando
    int cmd_argc;
    int status; //Donde guardaremos el status de finalización
    int c=0; //Contador de comandos ejecutados
    while(fgets(comando,MAX_CMD_LINE,file)!=NULL){ //Leemos una linea de tamaño MAX_CMD_LINE del fichero de texto al que se refuere "file" y lo guardamos en "comando"
        cmd_argv=parse_command(comando,&cmd_argc); //parseamos el comando (la linea leida) y guardamos el resultado en "cmd_argv"
        pidcmd = launch_command(cmd_argv,c);// mandamos a ejecutar el comando guardado en cmd_argv y recibimos el PID del hijo que lo ejecuta
        waitpid(pidcmd,&status,0);//esperamos a que termine la ejecución y guardamos en status el estado de fin del hijo
        if (WIFEXITED(status)) { 
            printf("@@ Command #%d terminated (pid: %d, status: %d)\n",c, pidcmd, WEXITSTATUS(status)); //escribimos la info recibida
        } else {
            printf("@@ Command #%d terminated (pid: %d) with unknown status\n",c, pidcmd);
        }
        c++;
        //Los comandos se ejecutan de uno en uno, por el waitpid que hay dentro de este bucle
    }
    fclose(file); //Cerramos el fichero
    for (int i = 0; cmd_argv[i] != NULL; i++) {  
        free(cmd_argv[i]);  // Free individual argument
    }

    free(cmd_argv); 
    return;
}
/*
    -------------- Funcion que se ejecuta en con los argumentos -s -b <file.txt> --------
    Tambien ejecuta los comandos escritos linea por linea en file.txt pero no espera a que acabe cada uno, los ejecuta todos seguidos y luego espera al final
*/
void ejecuta_fichero_b(char* file){
    //Abrimos fichero
    FILE* f;
    f = fopen(file,"r");
    if (!f) {
        perror("fopen");
        return;
    }
    // Mismas variable que en la anterior funcion
    char comando[MAX_CMD_LINE];
    char **cmd_argv;
    pid_t pidcmd;
    int cmd_argc;
    int c=0;
    //-------
    // Vector de elementos tipo proceso, donde se guardara el PID y numero de comando de cada comando que se ejecuta, el maximo de comandos es 256 en este caso
    struct proceso procesos[256];
    while(fgets(comando,MAX_CMD_LINE,f)!=NULL){ //leemos cada linea del fichero y la guardamos en "comando"
        cmd_argv=parse_command(comando,&cmd_argc); //parse del comando
        pidcmd = launch_command(cmd_argv,c); //ejecucion del comando
        procesos[c].pid=pidcmd; //guardamos la info del comando en la estructura, en el indice que corresponde
        procesos[c].command_number=c;
        c++;
        //En este caso NO esperamos a que acabe el anterior, NO hay wait dentro del bucle
    }
    fclose(f);
    int proc=c; //nos guardamos el numero de comandos ejecutados ( procesos creados )
    int status;
    while (c > 0) { //mientras queden procesos por acabar seguimos aqui
        pidcmd = wait(&status);// espermaos a que acabe un proceso y nos guardamos el PID
        if (pidcmd > 0) {
             for (int i = 0; i < proc; i++) { //Con el PID, buscamos en el vector donde guardamos la info de los procesos y buscamos el ha terminado para saber que numero de comando tenia #i
                if (procesos[i].pid == pidcmd) {//si pidcmd == procesos[i].pid es que lo hemos encontrado
                    printf("@@ Command #%d terminated (pid: %d, status: %d)\n", procesos[i].command_number, pidcmd, WEXITSTATUS(status));
                    break;//dejamos de buscar
                }
            }
            c--; //Ya ha terminado un proceso, reducimos los que nos quedan por esperar
        }
    }

    for (int i = 0; cmd_argv[i] != NULL; i++) {  
        free(cmd_argv[i]);  // Free individual argument
    }

    free(cmd_argv); 
    return;
}
//------!!!!!!!! Variables para usar GETOPT !!!!!!!!!!!---------
extern char *optarg;
extern int optind, opterr, optopt;
//------------------------------------------
int main(int argc, char *argv[]) {
    char **cmd_argv;
    int cmd_argc;
    int i;
    int  opt;
	struct options options;
    options.action=NONE_ACT;
  
    while ((opt = getopt(argc, argv, "x:s:b")) != -1)
	{
		switch (opt)
		{
		
		case 'x':
			options.cmds= optarg; //guarsamos en cmds la cadena (comando a ejecutar) que nos pasan como argumento en la opcion x
            options.action=X_CMD_ACT;
			break;
		case 's':
			options.input_file= optarg;
            if(options.action==NONE_ACT){
                options.action=FILE_X_ACT;
            }
			break;
        case 'b':
            options.action=FILE_BS_ACT;
            break;
		default:
			exit(EXIT_FAILURE);
		}
	}
    switch (options.action) //diferenciamos cada caso de ejcucion segun la opcion seleccionada
    {
        case X_CMD_ACT:
        cmd_argv=parse_command(options.cmds,&cmd_argc); //parsesamos comando
        launch_command(cmd_argv,-1);//ejecutamos comando
        wait(NULL);//esperamos a que acabe
        for (i = 0; cmd_argv[i] != NULL; i++) {  
            free(cmd_argv[i]);  // Free individual argument
        }
        free(cmd_argv); 
        break;
        case FILE_X_ACT:
            ejecuta_fichero(options.input_file); //ejecutamos la funcion correspondiente
        break;
        case FILE_BS_ACT:
            ejecuta_fichero_b(options.input_file);
        break;
        case NONE_ACT: //no se ha seleccionado nada
        fprintf(stderr, "Must indicate one of the following options: -x, -s, -b \n");
		
        break;
        default:
        break;
    }

    return EXIT_SUCCESS;
}