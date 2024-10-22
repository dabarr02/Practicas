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

void copy_link(char *orig, char *dest)
{
	printf("estoy en copylink\n");
	struct stat info;
	int n;
	n = lstat(orig,&info);
	int tam = info.st_size+1;
	char* ruta;
	ruta=malloc(sizeof(char)*tam);//memoria reservada
	n=readlink(orig,ruta,tam);
	ruta[tam-1]='\0';
	n=symlink(ruta,dest);
	if(n<0)
		exit(1);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct stat info;
	int n;
	n = lstat(argv[1],&info);
	if (S_ISREG(info.st_mode)){//regular
		copy_regular(argv[1],argv[2]);
	}else if(S_ISLNK(info.st_mode)){//enlace sim
		copy_link(argv[1],argv[2]);
	}else{
		printf("Tipo de archivo no soportado");
		exit(1);
	}
	return 0;
}
