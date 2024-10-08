#include <stdio.h>
#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include "defs.h"


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


student_t* parse_student(FILE* file,struct options* options,int* nr_entries){
	char line[MAX_PASSWD_LINE+1];
	char* token;
	char* lineptr;
	student_t* entries;
	student_t* cur_entry;
	token_id_t token_id;
	int entry_idx;
	int entry_count;
	int cur_line;
	
	char ch [5];
	//leemos el numero de entradas
	fgets(ch,sizeof(int), file);
	sscanf(ch,"%d",&entry_count);
	printf("Entries = %d",entry_count);
	//guardamos cuantas entradas hay
	
	entries=malloc(sizeof(student_t)*entry_count);
	/* zero fill the array of structures */
	memset(entries,0,sizeof(student_t)*entry_count);
	/* Parse file */
	entry_idx=0;
	cur_line=1;
	
	// esto no es un while es un for con entrycount?

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
					cur_entry->last_name=strdup(token);
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

int print_text_file(char *path,struct options* options)/* To be completed (part A) */
{
	FILE* file;
	int nr_entries;
	if ((file=fopen(path,"r"))==NULL) {
		fprintf(stderr, "%s could not be opened: ",path);
		perror(NULL);
		return 0;
		
	}
	student_t* estudiante = parse_student(file,options,&nr_entries);
	if (!estudiante)
		return EXIT_FAILURE;

	for(int i=0;i<nr_entries;i++){
		student_t* e=&estudiante[i];
		fprintf(stdout,"[Entry #%d]\n",i);
		fprintf(stdout,"\tstudent_id= %d\n\tNIF=%s\n\tfirst_name= %s\n\tlast_name= %s\n",e->student_id,e->NIF,e->first_name,e->last_name);
	}

	return 0;
}

int print_binary_file(char *path)
{
	/* To be completed  (part B) */
	return 0;
}


int write_binary_file(char *input_file, char *output_file)
{
	/* To be completed  (part C) */
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
	while ((opt = getopt(argc, argv, "hi:p")) != -1)
	{
		switch (opt)
		{
		case 'h':
			fprintf(stderr, "Usage: %s [ -h ]\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'i':
			options.input_file = optarg;
			break;
		case 'p':
			options.action=PRINT_TEXT_ACT;
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
		ret_code = print_text_file(options.input_file,&options);
		break;
	case WRITE_BINARY_ACT:
		/* Part B */
		ret_code = write_binary_file(options.input_file, options.output_file);
		break;
	case PRINT_BINARY_ACT:
		/* Part C */
		ret_code = print_binary_file(options.input_file);
		break;
	default:
		break;
	}
	exit(ret_code);
}
/// ESTO ES UNA PRUEBA DE GIT
