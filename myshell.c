#include <stdio.h>
#include "parser.h"
#include <sys/wait.h>
#include <stdlib.h> 
#include <unistd.h> // Para el execvp 

int
main(void) {
	char buf[1024];
	tline * line;
	int i,j;

	printf("msh> ");	
	while (fgets(buf, 1024, stdin)) {
	
		// Acceso al atributo de un puntero de forma sencilla -> 
		
		line = tokenize(buf);
		if (line==NULL) {
			continue;
		}
		if (line->redirect_input != NULL) {
			printf("redirección de entrada: %s\n", line->redirect_input);
		}
		if (line->redirect_output != NULL) {
			printf("redirección de salida: %s\n", line->redirect_output);
		}
		if (line->redirect_error != NULL) {
			printf("redirección de error: %s\n", line->redirect_error);
		}
		if (line->background) {
			printf("comando a ejecutarse en background\n");
		} 
		for (i=0; i<line->ncommands; i++) {
			printf("orden %d (%s):\n", i, line->commands[i].filename);
			for (j=0; j<line->commands[i].argc; j++) {
				printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
			}
		}
		
		// int execvp (const char *file , char *const argv[]); 
		
		char *argumento = line->commands[0].argv[0]; // Valor 0 del argv de commands, de tcommands del parser.h
		const char *fileName = line->commands[0].filename; 
		char *comando[] = {argumento,(char *) 0};  // Necesario que el argumento sea puntero a array 
		
		// Código del ejecuta
		
		pid_t pid;
		int status;

		pid = fork();

		if(pid<0){
			fprintf(stderr , "Falla el fork\n");
			return 1;
		} else if (pid == 0) { 
			execvp(fileName,comando);				// El primero es el nombre del programa y luego coje a partir del segundo. Mover le puntero a la siguiente. Char ** es el argv +1 , tambien vale &argv[1] 
			printf("Se ha producido un error\n");
			exit(1);
		} else {
			waitpid(pid,&status,0); // El 0
			exit(0);
		}
		printf("msh> ");	
	}
	return 0;
}
