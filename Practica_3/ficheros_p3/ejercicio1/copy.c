#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

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

int main(int argc, char *argv[])
{
	int fdd =creat(argv[2],0644);
	if(fdd==-1)
		exit(EXIT_FAILURE);
	int fdo = open (argv[1],O_RDONLY);
	if(fdo==-1)
		exit(EXIT_FAILURE);
	
	copy(fdo,fdd);
	return 0;
}
