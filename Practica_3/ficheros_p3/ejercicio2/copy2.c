#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
/*
  		   S_ISLNK    symbolic link
           S_ISREG      regular file
           S_ISBLK      block device
           S_IsDIR      directory
*/
//-------Fragmento identico a ejercicio 1-------------------
#define BUFSIZE 512
void copy(int fdo, int fdd)
{
	int nread;
	char buff[BUFSIZE];
	while((nread = read(fdo,buff,BUFSIZE))>0){
		if (write(fdd, buff, nread) < nread) {
			perror("write");
			close(fdd); close(fdo);
			exit(1);
		}		
	}
	if (nread < 0) {
		perror("read");
		close(fdd); close(fdo);
		exit(1);
	}
	close(fdd); close(fdo);
	exit(0);

}


void copy_regular(char *orig, char *dest)
{
	int fdd =creat(dest,0644);
	if(fdd==-1)
		exit(EXIT_FAILURE);
	int fdo = open (orig,O_RDONLY);
	if(fdo==-1)
		exit(EXIT_FAILURE);
	
	copy(fdo,fdd);
}
//-----------------------------------------------------------
//-----------Funcion para copiar enlaces simbólicos------------
void copy_link(char *orig, char *dest)
{

	struct stat info;
	int n;
	n = lstat(orig,&info);
	int tam = info.st_size+1; //nos da el tamaño del link
	char* ruta;//donde vamos a guarda el link
	ruta=malloc(sizeof(char)*tam);//memoria reservada
	n=readlink(orig,ruta,tam);//leemos la ruta que apunta el enlace symbolico
	ruta[tam-1]='\0';//añadimos el \0
	n=symlink(ruta,dest);//creamos el enlace simbolico
	if(n<0)//si fall,salimos con error
		exit(EXIT_FAILURE);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	struct stat info; //estructura que rellena la funcino lstat con la info del fichero
	int n;
	n = lstat(argv[1],&info); //obtenemos info
	if (S_ISREG(info.st_mode)){//Comprobamos si es regular
		copy_regular(argv[1],argv[2]);
	}else if(S_ISLNK(info.st_mode)){//Comprobamos si es enlace simbolico
		copy_link(argv[1],argv[2]);
	}else{
		printf("Tipo de archivo no soportado\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
