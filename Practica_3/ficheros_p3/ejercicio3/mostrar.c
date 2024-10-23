#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include "defs.h"
#include <fcntl.h>  
#include <stdio.h>
extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[])
{
	
	int ret_code, opt;
	struct options options;

	/* Initialize default values for options */
	options.n = -1;
	options.action = INICIO_ACT;
	options.input_file=NULL;
	ret_code = 0;

	/* Parse command-line options (incomplete code!) */
	while ((opt = getopt(argc, argv, "hn:e")) != -1)
	{
		switch (opt)
		{
		case 'h':
			fprintf(stderr, "Usage: %s [ -h ]\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'n':
			options.n=atoi(optarg);
			if(options.n<0){
				fprintf(stderr,"N cant be negative\n");
				exit(EXIT_FAILURE);
				}
			
			break;
		case 'e':
			options.action=FIN_ACT;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	if (options.n ==-1)
	{
		fprintf(stderr, "Must specify N bytes as an argument of -n\n");
		exit(EXIT_FAILURE);
	}
	options.input_file=argv[optind];
		if (options.input_file == NULL)
	{
		fprintf(stderr, "Must specify input file as an argument\n");
		exit(EXIT_FAILURE);
	}
	int fd=open(options.input_file,O_RDONLY);
	if(fd<0){
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}
	switch (options.action)
	{
	case INICIO_ACT:
		/* code */
		lseek(fd,options.n,SEEK_SET);
		break;
	case FIN_ACT:
		lseek(fd,-options.n,SEEK_END);
		break;
	default:
		fprintf(stderr, "Must specify an action -h for usage\n");
		exit(EXIT_FAILURE);
		break;
	}
	char c;
	int nread;
	int nwrite;
	while((nread=read(fd,&c,sizeof(char)))>0){//Leemos el fichero byte a byte
		if((nwrite=write(1,&c,sizeof(char)))<0){//escribimos por pantalla fd=1
			perror("Error al escribir");
			exit(EXIT_FAILURE);
		}

	}
	if(nread<0){
		perror("Error al leer archivo");
			exit(EXIT_FAILURE);
	}
	c='\n';
	if((nwrite=write(1,&c,sizeof(char)))<0){//escribimos por pantalla fd=1
			perror("Error al escribir");
			exit(EXIT_FAILURE);
		}
	return 0;
}
