#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
/**
 * Acabado
 */
int main(int argc, char* argv[])
{
	/* To be completed */
	FILE* file=NULL;
	if(argc>1){ //ha especificado archivo
		//abrimps archivo
		if ((file = fopen(argv[1],"w")) == NULL)
			err(2,"The input file %s could not be opened",argv[1]);
		char* str=NULL;
		for (int i = 2; i < argc; i++) {
			printf("Argumento %d: %s se ha escrito correctamente\n", i, argv[i]);
			str = argv[i];
			fprintf(file,"%s\n",str);
		}
		
	}else{
		fclose(file);
		exit(0);
	}
	fclose(file);
	return 0;
}
