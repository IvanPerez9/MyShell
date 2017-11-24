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

int funcionRedireccion ( char * entrada , char * salida , char * error );
void mycd ();

tline * line;


int main(void) {

	char buf[1024];
	int i,j;
	printf("msh> ");
	
	while (fgets(buf, 1024, stdin)) {

		// Acceso al atributo de un puntero de forma sencilla -> 

		line = tokenize(buf);		
		pid_t pid;
		int status;

		if (line==NULL) {
			continue;
		}

		if (line->redirect_input != NULL) {
			//printf("redirección de entrada: %s\n", line->redirect_input);
			funcionRedireccion ( line->redirect_input , NULL , NULL ); // ????????????????????????????????????????????????????
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

		// Código para 1 comando. Primero comprobar si es CD luego solo 1 comando cualquiera

		if(line->ncommands == 1){
			
			if (strcmp(line->commands[0].argv[0], "cd") == 0) { // Si es cd. Ejerccio del tema 3
				
				mycd(); // Subprograma de CD 
				
			} else { 	// Si es solo 1 comando pero NO es CD . Codigo Ejecuta tema 4 
			
				pid = fork();

				if(pid<0){
					fprintf(stderr , "Falla el fork %s\n", strerror(errno));
					exit(1);
				} else if (pid == 0) {  // Hijo 
					execvp(line->commands[0].filename,line->commands[0].argv);// El primero es el nombre del programa y luego coje a partir del segundo. Mover le puntero a la siguiente. Char ** es el argv +1 , tambien vale &argv[1] 
					// Sino error
					printf("Se ha producido un error\n");
				} else { // Padre
					// WIFEXITED(hijo) es 0 si el hijo ha terminado de manera anormal. Sino hace llamada a exit
					// WEXITATUS(hijo) devuelve el valor de la salia exit 
					wait(&status);
					if(WIFEXITED (status) != 0)
						if(WEXITSTATUS(status) != 0)
							printf("El comando se ha ejecutado correctamente\n");		
				}
			}

		} else {  // Son 2 comandos. line ncommands == 2  
			
			// Variable tuberia de memoria dinamica para los N comandos que haya
			// Variable pid para los descriptores de fichero, irá cambiando con los forks 
			// Contador para los bucles

			int ** tuberia = ( int ** ) malloc ((line->ncommands-1) * sizeof(int *));
			int * pid = (int *) malloc ((line-> ncommands-1) * sizeof (int));
			int i; 
			
			printf ("Hola\n");
			
			// Iniciar cada tuberia. Luego hacer free a cada una y la global . Inicializo los pipes (tuberia)
			
			for (i =0 ; i<line->ncommands-1 ; i++){
				tuberia [i] = malloc (2 * sizeof(int));	// 0 y 1 
				pipe(tuberia[i]);
				printf("Tuberia ini %d\n" , i);
			}
			
			// Bucle de busqueda hasta ncommands - 1
			// 1 entrada a tuberia, 0 salida de tuberia
			// 0 entrada estandar, 1 salida estandar 
			// dup2 redirije de old a new file ( descriptor de fichero ) 
			
			for(i=0; i< line->ncommands-1 ; i++){
				
				pid[i]	= fork ();
				
				printf ("Hola fork\n");
				
				if(pid[i] < 0){
					printf("El fork ha fallado");
					return 1;
				}else if(pid[i] == 0){
					
					printf("Hola bucle\n");
					
					if(i==0){	// Si es el primer hijo. Primer comando.
						printf("Hola bucle 0\n");
						close(tuberia[i][0]);
						dup2(tuberia[i][1] , 1);
						execvp(line->commands[i].filename,line->commands[i].argv);

						printf ("Hola primer hijo\n");

					} else if (i == line->ncommands -1 ){ // Es el último comando
						
						close(tuberia[i][1]);
						dup2(tuberia[i][0] , 0);
						execvp(line->commands[i].filename,line->commands[i].argv);

						printf ("Hola ultimo hijo\n");

					} else {							// El resto, comandos intermedios.
						
						close(tuberia[i][0]);
						close(tuberia[i-1][1]);
						dup2(tuberia[i][1] , 1);
						dup2(tuberia[i-1][0] , 0);
						execvp(line->commands[i].filename,line->commands[i].argv);

						printf ("Hola otros hijos\n");

					}

					execvp(line->commands[i].filename,line->commands[i].argv);// El primero es el nombre del programa y luego coje a partir del segundo. Mover le puntero a la siguiente. Char ** es el argv +1 , tambien vale &argv[1] 
					// Sino error
					printf("Se ha producido un error\n");
					exit(1);
			
				}
			}
			// Esperar a que termine todo
			for(i=0 ; i<line->ncommands; i++){
				// WIFEXITED(hijo) es 0 si el hijo ha terminado de manera anormal. Sino hace llamada a exit
				// WEXITATUS(hijo) devuelve el valor de la salia exit 
				wait(&status);
				if(WIFEXITED (status) != 0)
					if(WEXITSTATUS(status) != 0)
						printf("El comando se ha ejecutado correctamente\n");
			}
			
			// Cerrar los pipes 
			for(i=0 ; i<line->ncommands ; i++){
				close (tuberia[i][0]);
				close (tuberia[i][1]);
			}
			
			// Liberar memoria 
			for(i=0; i<line->ncommands; i++){
				free(tuberia[i]);	
			}
			free(tuberia);
			
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

void mycd (){
	
	char *dir; // Variable de directorios 
	char buffer[512];
	
	if(line->commands[0].argc > 2) // No puedo hacer un cd a 2 directorios 
		{
		  fprintf(stderr,"Uso: %s directorio\n", line->commands[0].argv[0]);
		}
	if (line->commands[0].argc == 1) // Si vale 1 , no me pasan nada, osea nombre del programa.
	{
		dir = getenv("HOME");
		if(dir == NULL)
		{
			fprintf(stderr,"No existe la variable $HOME\n");	
		}
	}else {
		dir = line->commands[0].argv[1];
	}
	
	// Comprobar si es un directorio.
	if (chdir(dir) != 0) { // Sino es distinto de 0 lo hace normal el chdir 
		fprintf(stderr,"Error al cambiar de directorio: %s\n", strerror(errno)); // Los errores a llamada al sistema siempre se guardan en errno, y strerror explica el porque de errno.
	}
	printf( "El directorio actual es: %s\n", getcwd(buffer,-1));
}



















































