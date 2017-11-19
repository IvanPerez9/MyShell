#include <stdio.h>
#include "parser.h"
#include <sys/wait.h>
#include <stdlib.h> 
#include <unistd.h> // Para el execvp 
#include <errno.h>
#include <wait.h> 
#include <string.h> 

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// gcc -Wall -Wextra myshell.c libparsher_64.a -o myshell 

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
			for (j=0; j<line->commands[i].argc; j++) {							// Recorre y dice cuales son los argumentos 
				printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
			}
		}
		
		// int execvp (const char *file , char *const argv[]); 
		// Código del ejecuta
		
		pid_t pid;
		int status;

		pid = fork();

		if(pid<0){
			fprintf(stderr , "Falla el fork %s\n", strerror(errno));
			exit(1);
		} else if (pid == 0) { 
			execvp(line->commands[0].filename,line->commands[0].argv);				// El primero es el nombre del programa y luego coje a partir del segundo. Mover le puntero a la siguiente. Char ** es el argv +1 , tambien vale &argv[1] 
			// Sino error
			printf("Se ha producido un error\n");
			exit(1);
		} else {
			// WIFEXITED(hijo) es 0 si el hijo ha terminado de manera anormal. Sino hace llamada a exit
			// WEXITATUS(hijo) devuelve el valor de la salia exit 
			wait(&status);
			if(WIFEXITED (status) != 0)
				if(WEXITSTATUS(status) != 0)
					printf("El comando se ha ejecutado correctamente\n");
			exit(0);		
		}
		printf("msh> ");	
	}
	return 0;
}

/*  entrada estandra (stdin 0) 
	salida estandar (stdout 1)
	error estandar (stderr 2)
*/

int funcionRedireccion ( char * entrada , char * salida , char * error ){
	
	FILE * fichero; // Variable para guardar las redirecciones 
	int ficheroError;
	int ficheroSalida;
	int ficheroEntrada;

	// Si es de entrada 
	if(entrada != NULL ) {
		fichero = fopen (entrada , "r") ; // Permiso read 
		if(fichero == NULL){
			fprintf( stderr , "%s : Error. %s\n" , entrada , strerror(errno)); // Mostrar error 
			return 1;
		} else {
			ficheroEntrada = fileno(fichero); // puntero del stream a descriptor de fichero . Para pasar a int 
			dup2(ficheroEntrada,0); // Redirreccion a 0, Entrada estandar 
			fclose(fichero);
		}	
	}
	
	// Si es de Salida
	if( salida != NULL ){
		fichero = fopen(salida , "w"); // Permiso Write
		if(fichero == NULL){
			fprintf (stderr , "%s : Error. %s\n" , salida , strerror(errno)); // Mostrar error 
			return 1;
		} else {
			ficheroSalida = fileno(fichero);
			dup2(ficheroSalida,1); // Redirreccion a 1, ESalida estandar 
			fclose(fichero);
		}
	}
	
	// Si es de error 
	if (error != NULL ){
		fichero = fopen (error , "w"); // Permiso write 
		if(fichero == NULL){
			fprintf(stderr , "%s : Error. %s\n" , error , strerror(errno)); // Mostrar error 
			return 1;
		}else {
			ficheroError = fileno(fichero);
			dup2(ficheroError,2); // Redirreccion a 2, Salida estandar 
			fclose(fichero);
		}
	}
	
	return 0;
}

int mycd(int argc, char *argv[]){ // Mycd del tema 3 ejercicios 
	char *dir;
	char buffer[512];
	
	if(argc > 2) // No puedo hacer un cd a 2 directorios 
	{
	  fprintf(stderr,"Uso: %s directorio\n", argv[0]);
	  return 1;
	}
	if (argc == 1) // Si vale 1 , no me pasan nada, osea nombre del programa. Por eso cd sin nada.
	{
		dir = getenv("HOME"); // Inicio en Home . Es como en shell $Home
		if(dir == NULL)
		{
		  fprintf(stderr,"No existe la variable $HOME\n");	
		}
	}
	else 
	{
		dir = argv[1];
	}
	// Comprobar si es un directorio
	if (chdir(dir) != 0) {
			fprintf(stderr,"Error al cambiar de directorio: %s\n", strerror(errno)); // Los errores a llamada al sistema siempre se guardan en errno, y strerror explica el porque de errno.
	}
	printf( "El directorio actual es: %s\n", getcwd(buffer,-1)); // comprobar si he cambiado de directorio. getcwd es como un pwd en shell 

	return 0;
}


















































