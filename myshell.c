#include <stdio.h>
#include "parser.h"
#include <sys/wait.h>
#include <stdlib.h> 
#include <unistd.h>
#include <errno.h>
#include <wait.h> 
#include <string.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

/*
* Autores: Iván Pérez Huete - Carlos Olmo Sahuquillo
*
* 11/30/17
*/

// gcc -Wall -Wextra myshell.c libparsher_64.a -o myshell 

int funcionRedireccion ( char * entrada , char * salida , char * error );
void mycd ();

#define CDCONTS "cd" // Constante para hacer cd 

tline * line;


int main(void) {

	char buf[1024];
	int i;
	pid_t pid;
	int ** tuberias;
	
	/* Señales, se tratan inmediatamente SIGINT == Ctrl+C y SIGQUIT == Ctrl + \ */
	// Ignoramos las señales
	signal(SIGINT , SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	
	
	
	printf("msh> ");
	
	while (fgets(buf, 1024, stdin)) {
		
		signal(SIGINT,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
		
		// Acceso al atributo de un puntero de forma sencilla -> 

		line = tokenize(buf);		
		int status;


		if (line==NULL) {
			continue;
		}

		if (line->redirect_input != NULL) {
			//printf("redirección de entrada: %s\n", line->redirect_input);
			//funcionRedireccion ( line->redirect_input , NULL , NULL ); 
			//freopen(line->redirect_input , "r" , stdin );
			return(1);
		}
		if (line->redirect_output != NULL) {
			//printf("redirección de salida: %s\n", line->redirect_output);
			freopen(line->redirect_output , "w" , stdout );
			return(1);
		}
		if (line->redirect_error != NULL) {
			//printf("redirección de error: %s\n", line->redirect_error);
			freopen(line->redirect_error , "w" , stderr );
			return(1);
		}
		if (line->background) {
			printf("comando a ejecutarse en background\n");
			return(1);
		} 

		// int execvp (const char *file , char *const argv[]); 

		// Para 1 solo comando, contampla CD 
		if(line->ncommands==1){
			
			//Señales, accion por defecto
				signal(SIGINT , SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
			
			// Comprueba si es cd
			if (strcmp(line->commands[0].argv[0], CDCONTS) == 0){// Comprueba si es el mandato es cd, llama a subprograma
				mycd();
			}else{ // Ejecuta el comando, como ejercicio de ejecuta 
				pid = fork();
				if (pid < 0) {  // Error fork
					fprintf(stderr, "Falló el fork().\n%s\n", strerror(errno));
					exit(1);
				}
				else if (pid == 0) { // Hace hijo 	
					
					execvp(line->commands[0].filename, line->commands[0].argv ); // Si pasa del exec , es que hay error
					fprintf(stderr, "Error al ejecutar el comando: %s\n", strerror(errno));
					exit(1);
				}else{ 	
					wait (&status);
					if (WIFEXITED(status) != 0)
						if (WEXITSTATUS(status) != 0)
							printf("El comando no se ejecutó correctamente\n");
				}
			}
		}else {// Para n Comandos
				
			// Variable tuberia de memoria dinamica para los N-1 comandos que haya ( una tuberia cada 2 comandos )
			// Variable pid para los descriptores de fichero, irá cambiando con los forks 
				
			tuberias=(int**) malloc ((line->ncommands-1)*sizeof(int*));
			for(i=0;i<line->ncommands-1;i++){
				tuberias[i]=(int*)malloc(2*sizeof(int));
				pipe(tuberias[i]);
				printf("Tuberia ini %d\n" , i);
			}
			int pids;
			for(i=0;i<line->ncommands;i++){
				
				//Señales, accion por defecto
					signal(SIGINT , SIG_DFL);
					signal(SIGQUIT, SIG_DFL);
				
				// Hace un fork() para cada hijo. El numero va cambiando con los forks
				pids=fork();				
				if(pids<0){ //error
					fprintf(stderr, "Falló el fork().\n%s\n", strerror(errno));
					exit(1);
				} else if (pids==0){ // Sino error hace hijos
					
					if(i==0){ //Primer hjijo
						close(tuberias[i][0]);
						dup2(tuberias[i][1],1);
						execvp(line->commands[i].filename, line->commands[i].argv );

						printf("Error al ejecutar el comando: %s\n", strerror(errno));
						exit(1);
					}else if(i==(line->ncommands-1)){// Utimo hijo
						close(tuberias[i-1][1]);
						dup2(tuberias[i-1][0],0);
						execvp(line->commands[i].argv[0], line->commands[i].argv);
						
						printf("Error al ejecutar el comando: %s\n", strerror(errno));
						exit(1);
					}else{	// Resto de hijos			
						close(tuberias[i][0]);	
						close(tuberias[i-1][1]);	
						dup2(tuberias[i-1][0],0);	
						dup2(tuberias[i][1],1);

						execvp(line->commands[i].argv[0], line->commands[i].argv);	

						printf("Error al ejecutar el comando: %s\n", strerror(errno));
						exit(1);
					}
				
				}else{	//Padre
					if(!(i==(line->ncommands-1))){ // Por esto la violacion del core
						close(tuberias[i][1]);
					}
				}
			}
			for(i=0;i<line->ncommands;i++){	
				waitpid(pids,&status,0);
				//fprintf(stderr,"waits: %s\n", strerror(errno));
			}
			//Liberamos la memoria reservada antes
			for(i=0;i<line->ncommands-1;i++){
				free(tuberias[i]);
			}
			free(tuberias);
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
			ficheroEntrada = fileno(fichero); // puntero del stream a descriptor de fichero . 
			dup2(ficheroEntrada,fileno(stdin)); // Redirreccion a 0, Entrada estandar 
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
			dup2(ficheroSalida,fileno(stdout)); // Redirreccion a 1, Salida estandar . Fileno para descriptor de fichero
			fclose(fichero);					// dup 2 solo usa int 
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
			dup2(ficheroError,fileno(stderr)); // Redirreccion a 2, Salida estandar 
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


















































