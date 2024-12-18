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
//Devuelve un string que es una copia de "original" equivale a strdup()
static inline char* clone_string(char* original)
{
	char* copy;
	copy=malloc(strlen(original)+1);
	strcpy(copy,original);
	return copy;
}
//-----Funcion para leer una cadena de caracteres de un binario devuelve un puntero apuntando al inicio de la cadena y su tamaño (hay que sumar 1 para que cuente con el /0)------------
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
//------------------------------------------------------------------------------
//-----------Funcion para leer de archivo binario------------------------------
student_t* read_student_binary_file(FILE* students, int* nr_entries){
	student_t* entries; //es lo que vamos a rellenar
	student_t* cur_entry; //Opcional?
	//leemos el numero de entradas
	fread(nr_entries,sizeof(int),1,students);
	entries=malloc(sizeof(student_t)*(*nr_entries)); //reservamos memoria para las entradas que vamos a leer
	/* zero fill the array of structures */
	memset(entries,0,sizeof(student_t)*(*nr_entries));
	char* str; //Donde vamos a ir cargando lo que leemos
	for (int i = 0; i < *nr_entries; i++) //hacemos nr_entries iteraciones
	{	
		cur_entry=&entries[i]; //seleccionamos la entrada i
		int aux; //Donde leemos el entero (ID)
		fread(&aux,sizeof(int),1,students);//leemos ID
		cur_entry->student_id=aux;
		//leemos NIF
		str =loadstr(students,&aux);
		//copiamos el nif en la entrada cur_entry (la entrada i)
		strncpy(cur_entry->NIF, str, sizeof(cur_entry->NIF)); //Podemos usar strncopy porque cur_entry->NIF tiene tamaño reservado (es char[10])
		free(str);
		//leemos Nombre
		str=loadstr(students,&aux);
		cur_entry->first_name=strdup(str); //Usamos strdup porque la memoria de first_name no esta inicializada
		free(str);
		//Leemos apellido
		str=loadstr(students,&aux);
		cur_entry->last_name=strdup(str); //lo mismo que el nombre
		free(str);
	}
	
	return entries;	//Devolvemos las entradas que hemos leido


}
// ------ Funcion para leer de archivo de texto-----------
student_t* read_student_text_file(FILE* file,int* nr_entries){
	char line[MAX_PASSWD_LINE+1];
	char* token; //donde se alamacenará cada trozo separado
	char* lineptr;//puntero de inicio de linea
	student_t* entries; //entradas a rellenar y devolver
	student_t* cur_entry;//entrada actual
	token_id_t token_id; //enumerado para saber porque campo vamos
	int entry_idx;
	int entry_count; //para guardar cuantos alumnos (entradas) hay en el fichero
	int cur_line;
	
	char ch [MAX_PASSWD_LINE];
	//leemos el numero de entradas
	fgets(ch,sizeof(int), file);
	sscanf(ch,"%d",&entry_count);
	printf("Entries = %d \n",entry_count);
	//guardamos cuantas entradas hay
	//reservamos memoria para las entradas
	entries=malloc(sizeof(student_t)*entry_count);
	/* zero fill the array of structures */
	memset(entries,0,sizeof(student_t)*entry_count);
	/* Parse file */
	entry_idx=0; //llevamos la cuenta de porque entrada vamos
	cur_line=1;
	
	
	//Leemos linea a linea hasta que se acabe
	while (fgets(line, MAX_PASSWD_LINE + 1, file) != NULL){
		lineptr=line;//Guardamos en line_ptr el puntero de inicio de la linea leida
		token_id=STUDENT_ID_IDX;//lo primero que leemos es el id
		cur_entry=&entries[entry_idx];//Seleccionamos la entrada donde vamos a guardar los datos (i)
		while((token = strsep(&lineptr, ":"))!=NULL) //Separamos la linea en trozos separados por :
		{
			switch (token_id){//Primer trozo es el ID
				case STUDENT_ID_IDX:
					sscanf(token,"%d",&cur_entry->student_id);//Leemos el id y lo guardamos en la entrada actual
					printf("El token ID es: %d\n",cur_entry->student_id);
				break;
				case NIF_IDX://El siguiente campo es el DNI
					strcpy(cur_entry->NIF,token);//Usamos strcpy porque NIF ya esta reservada la memoria, (Copiamos token en NIF)
				break;
				case FIRST_NAME_IDX: //Nombre
					
				cur_entry->first_name=strdup(token);//Usamos strdup porque no esta reservada la memoria para nombre
				break;
				case LAST_NAME_IDX:
					int len= strlen(token);
					if(token[len-1]=='\n'){	//Comprobamos si el ultimo caracter es un \n, no lo queremos guardar
						cur_entry->last_name=malloc(sizeof(char)*(len));//Reservamos memoria para el apellido
						strncpy(cur_entry->last_name,token,len-1);//Copiamos todo menos el \n 
						cur_entry->last_name[len-1]='\0';//añadimos el \0 al final
					}else{//si no hay salto de linea no hacemos nada
						cur_entry->first_name=strdup(token);//Copiar token en apellido
					}
				break;
				default:
				break;
			} 
			token_id++;//Aumentamos el contandor de token por el que vamos
		}
		if (token_id!=NR_FIELDS_STUDENT) {//Esto pasa si no hemos leido todos los campos de una linea
			fprintf(stderr, "Could not process all tokens from line %d\n",entry_idx+1);
			return NULL;
		}
		cur_line++;
		entry_idx++;
	}
	(*nr_entries)=entry_count;
	return entries;
}
//---------Funcion para imprmir leyendo de un fichero de texto---------------
int print_text_file(char *path)/* To be completed (part A) */
{
	FILE* file;
	int nr_entries;
	if ((file=fopen(path,"r"))==NULL) { //Abrimos el fichero
		fprintf(stderr, "%s could not be opened: ",path);
		perror(NULL);
		return 0;
		
	}
	student_t* estudiante = read_student_text_file(file,&nr_entries); //Leemos el fichero y obtenemos las entradas de cada estudiante
	if (!estudiante) {//Si esta vacio ERROR
		fclose(file);//cerramos el fichero
		return EXIT_FAILURE;
	}
	//Imprimimos por pantalla todas las entradas
	for(int i=0;i<nr_entries;i++){
		student_t* e=&estudiante[i]; //seleccionamos entrada a imprimir
		fprintf(stdout,"[Entry #%d]\n",i);//imprimimos en la consola (stdout)
		fprintf(stdout,"\tstudent_id=%d\n\tNIF=%s\n\tfirst_name=%s\n\tlast_name=%s\n",e->student_id,e->NIF,e->first_name,e->last_name);
		// \t es para las tabulaciones
	}
	fclose(file);//cerramos el fichero
	return 0;
}
//---------Funcion para imprimir leyendo de Binario--------------
int print_binary_file(char *path)
{
	//----------Identico a la funcino anterior print_text_file-------------
	/* To be completed  (part C) */
	FILE* file;
	int nr_entries;
	if ((file=fopen(path,"r"))==NULL) {
		fprintf(stderr, "%s could not be opened: ",path);
		perror(NULL);
		return 0;
		
	}
	student_t* estudiante = read_student_binary_file(file,&nr_entries); //Leemos las entradas desde binario
	if (!estudiante){
		fclose(file);//cerramos el fichero
		return EXIT_FAILURE;
	}
	
	for(int i=0;i<nr_entries;i++){
		student_t* e=&estudiante[i];
		fprintf(stdout,"[Entry #%d]\n",i);
		fprintf(stdout,"\tstudent_id=%d\n\tNIF=%s\n\tfirst_name=%s\n\tlast_name=%s\n",e->student_id,e->NIF,e->first_name,e->last_name);
	}

	fclose(file);//cerramos el fichero
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
//--------Funcion para escribir en archivo binario las entradas desde un fichero de texto -----
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
		fclose(texto);//cerramos el fichero
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
	//Escribimos el numero de entradas en el binario
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
//------Funcion que devuelve una entrada de tipo estudiante a partir de una cadena de caracteres-----------
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
//--Funcion para añadir un nuevo estudiante en un archivo binario <input file>-------------------
int anadir_bin(char *input_file,char** registros){
	int cont=0;
	FILE* file;
	student_t estudiante;
	file=fopen(input_file,"r+"); //Abrimos el fichero
	fseek(file, 0, SEEK_END);//nos posicionamos al final para añadir los nuevos elementos
	//bucle para escribir al final
	while(registros[cont]!=NULL){ //Seguimos añadiendo mientras queden registros
		estudiante=parse_student(registros[cont]); //Parseamos un registro y recibimos la entrada de estudiante rellena
		cont++;//Aumentamos contandor y lo usamos para llevar la cuenta de cuantos registros hemos añadido
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
	fseek(file, 0, SEEK_SET);//Nos colocamos al principio del fichero otra vez
	int c;
	fread(&c, sizeof(int), 1, file);//leemos c
	c=c+cont; //Actualizamos el contador de registros del fichero
	fseek(file, 0, SEEK_SET);//Volvemos al inicio para sobreescribir el dato de numero de entradas del fichero
	fwrite(&c, sizeof(int), 1, file);//escribimos c+cont (Sobreescribimos)
	fclose(file);
	return 0;
}
//------Funcion para añadir registros a un fichero de texto----------------
int anadir_txt(char *input_file,char** registros){
	int cont=0;
	FILE* file;
	student_t estudiante;
	file=fopen(input_file,"r+"); //Abrimo el fichero donde escribiremos
	//bucle para escribir al final
	fseek(file, 0, SEEK_END);//Nos ponemos al final
	while(registros[cont]!=NULL){
		estudiante=parse_student(registros[cont]); //Parseamos el registro que toca
		fprintf(file,"\n%d:%s:%s:%s",estudiante.student_id,estudiante.NIF,estudiante.first_name,estudiante.last_name); //Escrbimios el registro donde toca
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
