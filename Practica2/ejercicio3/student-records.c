#include <stdio.h>
#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include "defs.h"
#include <fcntl.h>  


/* Assume lines in the text file are no larger than 100 chars */
#define MAXLEN_LINE_FILE 100

extern char *optarg;
extern int optind, opterr, optopt;

static inline char* clone_string(char* original)
{
	char* copy;
	copy=malloc(strlen(original)+1);
	strcpy(copy,original);
	return copy;
}
char *loadstr(FILE *file, int *contar) {
    char c;
    *contar = 0;

    // Contar la longitud de la cadena hasta el carácter nulo
    while (fread(&c, sizeof(char), 1, file) == 1) {
        if (c == '\0') {
            break; // Se encontró el carácter nulo
        }
        (*contar)++;
    }

    // Si no hay caracteres leídos, retornar NULL
    if (*contar == 0) {
        return NULL;
    }

    // Asignar memoria para la cadena +1 para el carácter nulo
    char *str = (char *)malloc((*contar + 1) * sizeof(char));
    if (str == NULL) {
        perror("Error al asignar memoria");
        return NULL; // Manejo de error de asignación
    }

    // Regresar al inicio de la cadena leída
    fseek(file, -(*contar + 1), SEEK_CUR); // Mover el puntero para leer la cadena
    fread(str, sizeof(char), *contar, file); // Leer la cadena
	fseek(file, 1, SEEK_CUR);
    str[*contar] = '\0'; // Añadir carácter nulo al final

    return str;	
}
student_t* read_student_binary_file(FILE* students, int* nr_entries){
	student_t* entries; //es lo que vamos a rellenar
	student_t* cur_entry; //Opcional?
	//leemos el numero de entradas
	fread(nr_entries,sizeof(int),1,students);
	entries=malloc(sizeof(student_t)*(*nr_entries));
	/* zero fill the array of structures */
	memset(entries,0,sizeof(student_t)*(*nr_entries));
	char* str;
	for (int i = 0; i < *nr_entries; i++) //hacemos nr_entries iteraciones
	{	
		cur_entry=&entries[i];
		int aux;
		fread(&aux,sizeof(int),1,students);//leemos ID
		cur_entry->student_id=aux;
		//leemos NIF
		str =loadstr(students,&aux);
		strncpy(cur_entry->NIF, str, sizeof(cur_entry->NIF));
		free(str);
		//leemos Nombre
		str=loadstr(students,&aux);
		cur_entry->first_name=strdup(str);
		free(str);
		//Leemos apellido
		str=loadstr(students,&aux);
		cur_entry->last_name=strdup(str);
		free(str);
	}
	
	return entries;	


}

student_t* read_student_text_file(FILE* file,int* nr_entries){
	char line[MAX_PASSWD_LINE+1];
	char* token;
	char* lineptr;
	student_t* entries;
	student_t* cur_entry;
	token_id_t token_id;
	int entry_idx;
	int entry_count;
	int cur_line;
	
	char ch [MAX_PASSWD_LINE];
	//leemos el numero de entradas
	fgets(ch,sizeof(int), file);
	sscanf(ch,"%d",&entry_count);
	printf("Entries = %d \n",entry_count);
	//guardamos cuantas entradas hay
	
	entries=malloc(sizeof(student_t)*entry_count);
	/* zero fill the array of structures */
	memset(entries,0,sizeof(student_t)*entry_count);
	/* Parse file */
	entry_idx=0;
	cur_line=1;
	
	

	while (fgets(line, MAX_PASSWD_LINE + 1, file) != NULL){
		lineptr=line;
		token_id=STUDENT_ID_IDX;
		cur_entry=&entries[entry_idx];
		while((token = strsep(&lineptr, ":"))!=NULL) //solo hay que leer cuatro campos
		{
			switch (token_id){
				case STUDENT_ID_IDX:
					sscanf(token,"%d",&cur_entry->student_id);
					printf("El token ID es: %d\n",cur_entry->student_id);
				break;
				case NIF_IDX:
					strcpy(cur_entry->NIF,token);
				break;
				case FIRST_NAME_IDX:
					
				cur_entry->first_name=strdup(token);
				break;
				case LAST_NAME_IDX:
					int len= strlen(token);
					if(token[len-1]=='\n'){	
						cur_entry->last_name=malloc(sizeof(char)*(len));
						strncpy(cur_entry->last_name,token,len-1);
						cur_entry->last_name[len-1]='\0';
					}else{
						cur_entry->first_name=strdup(token);
					}
				break;
				default:
				break;
			} 
			token_id++;
		}
		if (token_id!=NR_FIELDS_STUDENT) {
			fprintf(stderr, "Could not process all tokens from line %d\n",entry_idx+1);
			return NULL;
		}
		cur_line++;
		entry_idx++;
	}
	(*nr_entries)=entry_count;
	return entries;
}

int print_text_file(char *path)/* To be completed (part A) */
{
	FILE* file;
	int nr_entries;
	if ((file=fopen(path,"r"))==NULL) {
		fprintf(stderr, "%s could not be opened: ",path);
		perror(NULL);
		return 0;
		
	}
	student_t* estudiante = read_student_text_file(file,&nr_entries);
	if (!estudiante)
		return EXIT_FAILURE;

	for(int i=0;i<nr_entries;i++){
		student_t* e=&estudiante[i];
		fprintf(stdout,"[Entry #%d]\n",i);
		fprintf(stdout,"\tstudent_id=%d\n\tNIF=%s\n\tfirst_name=%s\n\tlast_name=%s\n",e->student_id,e->NIF,e->first_name,e->last_name);
	}

	return 0;
}

int print_binary_file(char *path)
{
	/* To be completed  (part C) */
	FILE* file;
	int nr_entries;
	if ((file=fopen(path,"r"))==NULL) {
		fprintf(stderr, "%s could not be opened: ",path);
		perror(NULL);
		return 0;
		
	}
	student_t* estudiante = read_student_binary_file(file,&nr_entries);
	if (!estudiante)
		return EXIT_FAILURE;

	for(int i=0;i<nr_entries;i++){
		student_t* e=&estudiante[i];
		fprintf(stdout,"[Entry #%d]\n",i);
		fprintf(stdout,"\tstudent_id=%d\n\tNIF=%s\n\tfirst_name=%s\n\tlast_name=%s\n",e->student_id,e->NIF,e->first_name,e->last_name);
	}


	return 0;
}
/*
char* format_str(char* str,int* len){
	size_t longitud = strlen(str); //longitud original
	size_t longitud_o=longitud+1; //dejamos hueco para el \0
	char *cadena_con_nulo = malloc(longitud_o); //creamos la cadena nueva
    if (cadena_con_nulo == NULL) {
        perror("Error al asignar memoria");
        return NULL;
    }
	*len=longitud_o;
	strcpy(cadena_con_nulo, str); //strcpy copia str en cadena_con_nulo str y añade \0
	return cadena_con_nulo;
}*/
int write_binary_file(char *input_file, char *output_file)
{
	/* To be completed  (part B) */
	FILE* texto; //file para parseo de texto
	int nr_entries;
	if ((texto=fopen(input_file,"r"))==NULL) {
		fprintf(stderr, "%s could not be opened: ",input_file);
		perror(NULL);
		return 0;
		
	}
	//Obtenemos las entradas y numero de entradas del archivo .txt
	student_t* estudiante = read_student_text_file(texto,&nr_entries); 
	if (!estudiante){//si falla exit
		return EXIT_FAILURE;
	}
	fclose(texto);
	//creamos el fichero con permisos
	int fd = creat(output_file, 0700);
    if (fd == -1) {
        perror("Error al crear el fichero");
        return 1;
    }

    //usamos fdopen para usar un FILE y poder usa fwrite
    FILE *binario = fdopen(fd, "wb");
	    if (binario == NULL) {
        perror("Error al abrir el descriptor como FILE*");
        close(fd);
        return 1;
    }
	//--------------Tenemos las entradas, numero de entradas y hemos creado y abierto el fichero----------------------
	//Escribimos el numero de entradas
	fwrite(&nr_entries,sizeof(int),1,binario);
	for(int i=0;i<nr_entries;i++){ //bucle para recorrer las entradas  su NIF, su nombre y apellido
		student_t* e=&estudiante[i];
		//escribimos el id
		fwrite(&e->student_id,sizeof(int),1,binario);
		//NIF
		fwrite(e->NIF,sizeof(char),strlen(e->NIF)+1,binario);
		//Nombre
		fwrite(e->first_name,sizeof(char),strlen(e->first_name)+1,binario);
		//Apellidos
		fwrite(e->last_name,sizeof(char),strlen(e->last_name)+1,binario);
	}
	fclose(binario);
	return 0;
}
student_t parse_student(char* regis){
	student_t ret;
	token_id_t token_id=0;
	char* token;
	while((token = strsep(&regis, ":"))!=NULL) //
		{
			switch (token_id){
				case STUDENT_ID_IDX:
					sscanf(token,"%d",&ret.student_id);
				break;
				case NIF_IDX:
					strcpy(ret.NIF,token);
				break;
				case FIRST_NAME_IDX:
					
				ret.first_name=strdup(token);
				break;
				case LAST_NAME_IDX:
					ret.last_name=strdup(token);
				break;
				default:
				break;
			} 
			token_id++;
		}
		if (token_id!=NR_FIELDS_STUDENT) {
			fprintf(stderr, "Could not process all tokens from line\n");
			exit(EXIT_FAILURE);

		}
	return ret;

}
int anadir_bin(char *input_file,char** registros){
	int cont=0;
	FILE* file;
	student_t estudiante;
	file=fopen(input_file,"r+");
	fseek(file, 0, SEEK_END);
	//bucle para escribir al final
	while(registros[cont]!=NULL){
		estudiante=parse_student(registros[cont]);
		cont++;
		//escribimos el id
		fwrite(&estudiante.student_id,sizeof(int),1,file);
		//NIF
		fwrite(estudiante.NIF,sizeof(char),strlen(estudiante.NIF)+1,file);
		//Nombre
		fwrite(estudiante.first_name,sizeof(char),strlen(estudiante.first_name)+1,file);
		//Apellidos
		fwrite(estudiante.last_name,sizeof(char),strlen(estudiante.last_name)+1,file);
	}
	//tenemos que actualizar el primer entero
	fseek(file, 0, SEEK_SET);
	int c;
	fread(&c, sizeof(int), 1, file);//leemos c
	c=c+cont;
	fseek(file, 0, SEEK_SET);
	fwrite(&c, sizeof(int), 1, file);//escribimos c+cont
	fclose(file);
	return 0;
}
int anadir_txt(char *input_file,char** registros){
	int cont=0;
	FILE* file;
	student_t estudiante;
	file=fopen(input_file,"r+");
	//bucle para escribir al final
	fseek(file, 0, SEEK_END);
	while(registros[cont]!=NULL){
		estudiante=parse_student(registros[cont]);
		fprintf(file,"\n%d:%s:%s:%s",estudiante.student_id,estudiante.NIF,estudiante.first_name,estudiante.last_name);
		cont++;
	}
	//tenemos que actualizar el primer entero
	fseek(file, 0, SEEK_SET);
	int c;
	fscanf(file, "%d", &c);//leemos c
	c=c+cont;
	fseek(file, 0, SEEK_SET);
	fprintf(file, "%d", c);//escribimos c modificado
	fclose(file);
	return 0;
}
int main(int argc, char *argv[])
{
	int ret_code, opt;
	struct options options;

	/* Initialize default values for options */
	options.input_file = NULL;
	options.output_file = NULL;
	options.action = NONE_ACT;
	ret_code = 0;

	/* Parse command-line options (incomplete code!) */
	while ((opt = getopt(argc, argv, "hi:po:ba")) != -1)
	{
		switch (opt)
		{
		case 'h':
			fprintf(stderr, "Usage: %s [ -h ]\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'i':
			options.input_file = optarg;
			break;
		case 'o':
			options.output_file = optarg;
			options.action=WRITE_BINARY_ACT;
			break;
		case 'p':
			options.action=PRINT_TEXT_ACT;
			break;
		case 'b':
			options.action=PRINT_BINARY_ACT;
			break;
		case 'a':
			options.action=ANADIR;
			break;

		/**
		 **  To be completed ...
		 **/

		default:
			exit(EXIT_FAILURE);
		}
	}

	if (options.input_file == NULL)
	{
		fprintf(stderr, "Must specify one record file as an argument of -i\n");
		exit(EXIT_FAILURE);
	}

	switch (options.action)
	{
	case NONE_ACT:
		fprintf(stderr, "Must indicate one of the following options: -p, -o, -b \n");
		ret_code = EXIT_FAILURE;
		break;
	case PRINT_TEXT_ACT:
		/* Part A */
		//llamamos a la funcion del apartado A
		ret_code = print_text_file(options.input_file);
		break;
	case WRITE_BINARY_ACT:
		/* Part B */
		ret_code = write_binary_file(options.input_file, options.output_file);
		break;
	case PRINT_BINARY_ACT:
		/* Part C */
		ret_code = print_binary_file(options.input_file);
		break;
	case ANADIR:
		int tam= strlen(options.input_file);
		if(options.input_file[tam-1]=='t'){
			//añadimos al txt
			options.registros=&argv[optind]; //guardamos los registros a escribir
			ret_code=anadir_txt(options.input_file,options.registros);
		}else{
			//añadimos al bin
			options.registros=&argv[optind];//guardamos los registros a escribir 
			ret_code=anadir_bin(options.input_file,options.registros);
		}
		break;
	default:
		break;
	}
	exit(ret_code);
}
/// ESTO ES UNA PRUEBA DE GIT
