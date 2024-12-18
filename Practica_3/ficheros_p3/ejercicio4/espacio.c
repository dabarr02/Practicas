#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>  
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

/* Forward declaration */
int get_size_dir(char *fname, size_t *blocks);

/* Gets in the blocks buffer the size of file fname using lstat. If fname is a
 * directory get_size_dir is called to add the size of its contents.
 */
int get_size(char *fname, size_t *blocks)
{
	struct stat info;
	int n;
	n = lstat(fname,&info); //guardamos en info la informacion del fichero
	if (S_ISREG(info.st_mode)){//Si es fichero Regular actualizamos con su numero de bloques (st_blocks)
		*blocks += info.st_blocks;
	}else if(S_ISDIR(info.st_mode)){//Si es un Dir llamamos a la funcion get_size_dir
		get_size_dir(fname,blocks);
	}else{
		printf("Tipo de archivo no soportado");
		exit(1);
	}
	return 0;
}


/* Gets the total number of blocks occupied by all the files in a directory. If
 * a contained file is a directory a recursive call to get_size_dir is
 * performed. Entries . and .. are conveniently ignored.
 */
int get_size_dir(char *dname, size_t *blocks)
{
	struct stat info;
	int n;
	struct dirent* info_dir;
	n = lstat(dname,&info); //guardamos en info la informacion del directorio
	*blocks += info.st_blocks;//se suman los bloques que ocupa el propio DIR
	DIR* directorio = opendir(dname); //abrimos directorio
	if(directorio==NULL){
		perror("Abrir directorio");
		exit(EXIT_FAILURE);
	}
	while((info_dir=readdir(directorio))!=NULL){// leemos directorio mientras la entrada sea distinta de NULL
		if(strcmp(info_dir->d_name, ".") == 0 || strcmp(info_dir->d_name, "..") == 0){ //evitamo recursion infinita, no podemos meternos en los directorios padre (..) o propio (.)
			continue;
		}
		if(info_dir->d_type == DT_DIR){ //Si la entrada en Dir hacemo recursion

			get_size_dir(dname,blocks);

		}else if(info_dir->d_type == DT_REG){// Si la entrada es Regular, llamamos a get_size
			get_size(info_dir->d_name,blocks);
		}
	}
	return 0;
}

/* Processes all the files in the command line calling get_size on them to
 * obtain the number of 512 B blocks they occupy and prints the total size in
 * kilobytes on the standard output
 */
int main(int argc, char *argv[])
{
	size_t blocks;
	for (int i = 1; i < argc; i++)
	{	
		blocks=0;
		char buffer[20];
		get_size(argv[i],&blocks); //llamamos a get_size para saber los bloques
		int sz_kb = blocks/2; //como los bloques son de 512B y queremos saber cuantos 1024B hay dividimos por 2
    	// Convertimos el entero a string y le agregamos la "k" y el nombre del fichero
		sprintf(buffer, "%dk %s\n",sz_kb,argv[i]);
		// Usamos write para escribir el contenido de buffer
		write(1, buffer, strlen(buffer));
	
	}
	
	return 0;
}
