#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h> 
#include <stdbool.h>
/*
	Acabado

*/
/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */

char * progname;

char *loadstr(FILE *file,int *contar)
{
	/* To be completed */
	int c;
	bool salto=false;
	*contar=0;
	
	while ((fread(&c,sizeof(char),1,file)) != 0&&!salto) {
		/* Print byte to stdout */
		if(c=='\0'){
			salto=true;
		}
		*contar=*contar+1;
	}
	char *str ;
	if(*contar>0){
		str = (char*) malloc(*contar * sizeof(char));
		fseek(file,-*contar,SEEK_CUR);
		fread(str,sizeof(char),*contar,file);
	}else{
		return NULL;
	}
	return str;	
}


int main(int argc, char *argv[])
{
	/* To be completed */

	FILE* file=NULL;
	
	char* title=NULL;
	

	progname = argv[0];

	

	/* Parse command-line options */
	title=argv[1];
	printf("Abriendo: %s\n",title);
	file=fopen(title,"r");
	if (!file){
		perror("out:"); 
		exit(1);
	}
	int * contar =(int*)malloc(sizeof(int));
	char* str = loadstr(file,contar);
	
	while(str!=NULL){
		int ret=0;
		//ret=fwrite(str,sizeof(char),*contar,stdout);
		write(1, str, *contar * sizeof(char));
		if (ret==EOF){
			fclose(file);
			err(3,"fwrite) failed!!");
		}
		str=loadstr(file,contar);
	}
	fclose(file);
	free(str);
	free(contar);
	return 0;

}
