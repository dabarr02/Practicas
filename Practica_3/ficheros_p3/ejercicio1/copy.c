#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 512
void copy(int fdo, int fdd)
{
	int nread;
	char buff[BUFSIZE];
	while((nread = read(fdo,buff,BUFSIZE))>0){ //Leemos en bloques de 512, almacenando en buff. El bucle sigue mientras no de error (-1) y no lleguemos a EOF (0)
		if (write(fdd, buff, nread) < nread) {//Escribimos lo leido. Da error si el tamaño escrito (ret de write) es menor que lo que debe (nread) 
			perror("write");
			close(fdd); close(fdo);
			exit(EXIT_FAILURE);
		}		
	}
	if (nread < 0) { //Error al leer
		perror("read");
		close(fdd); close(fdo);
		exit(EXIT_FAILURE);
	}
	close(fdd); close(fdo);
	exit(EXIT_SUCCESS);

}

int main(int argc, char *argv[])
{
	if (argc != 3) {
        fprintf(stderr, "Uso: %s <nombre_archivo_origen> <nombre_archivo_destino>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
	int fdd =creat(argv[2],0644); //creamos archivo destino
	if(fdd==-1)
		exit(EXIT_FAILURE);
	int fdo = open (argv[1],O_RDONLY); //abrimos archivo origen
	if(fdo==-1)
		exit(EXIT_FAILURE);
	
	copy(fdo,fdd);
	return 0;
}
