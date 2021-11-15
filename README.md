# SO-myshell

![Bash Shell](https://badges.frapsoft.com/bash/v1/bash.png?v=103)

<b>[ESP]</b> Práctica 2 de Sistemas Operativos. En esta práctica se abordará el problema de implementar un programa que actúe como intérprete de mandatos. El minishell a implementar debe interpretar y ejecutar mandatos leyéndolos de la entrada estándar.

<b>[EN]</b> Second project of Operating Systems. The implemented minishell pretends to works as the original.The idea is to provide a basic personal minishell.

## Compilación // How to compile this proyect: 

<b>[ESP]</b>
Usamos Wall y Wextra como opciones de gcc para filtar y habilitar los warnings de todo tipo.

<b>Wall: </b>Habilita todos los avisos de construcciones que el usuario considere cuestionables, y que son faciles de evitar, modificar o prevenir.

<b> Wextra: </b>Habilita algunos avisos extra que no son habilitados con -Wall. (Esta opcion se suele llamar tambien con -W solo, pero -Wextra es más representativa.)
```shell
$ make
gcc -Wall -Wextra myshell.c libparsher_64.a -o myshell 
```

El resto de ficheros <i>libparsher_64.a</i> son usados para dar la funcionalidad a la shell. <b>No son creados por mí. </b>

-------------------------------
<b>[EN]</b>
We use Wall and Wextra as options to request or suppress warnings.

<b>Wall: </b>This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros.

<b> Wextra: </b>This enables some extra warning flags that are not enabled by -Wall. (This option used to be called -W. The older name is still supported, but the newer name is more descriptive.)
```shell
$ make
gcc -Wall -Wextra myshell.c libparsher_64.a -o myshell 
```

The rest of files <i>libparsher_64.a</i> are used to provide our shell the reading and interpreting functionality of the introduced commands. <b>These files were not created by me. </b>

## Normas // Rules 
* __En definitiva debe ser capaz de:__
  * Ejecutar una secuencia de uno o varios mandatos separados por el carácter ‘|’.
  * Permitir redirecciones:
  * Entrada: < fichero. Sólo puede realizarse sobre el primer mandato del pipe.
  * Salida: > fichero. Sólo puede realizarse sobre el último mandato del pipe.
  * Error: >& fichero. Sólo puede realizarse sobre el último mandato del pipe.
  * Permitir la ejecución en background de la secuencia de mandatos si termina con el carácter ‘&’. Para ello, el
  minishell debe mostrar el pid del proceso por el que espera entre corchetes, y no bloquearse por la ejecución
  de dicho mandato (es decir no debe esperar a mostrar el prompt a su terminación).
  A grandes rasgos, el programa tiene que hacer lo siguiente:
  * Mostrar en pantalla un prompt (los símbolos msh> seguidos de un espacio).
  * Leer una línea del teclado.
  * Analizarla utilizando la librería parser (ver apéndice).
  * Ejecutar todos los mandatos de la línea a la vez creando varios procesos hijo y comunicando unos con otros
  con las tuberías que sean necesarias, y realizando las redirecciones que sean necesarias. En caso de que no se
  ejecute en background, se espera a que todos los mandatos hayan finalizado para volver a mostrar el prompt
  y repetir el proceso.
  Teniendo en cuenta lo siguiente:
  * Si la línea introducida no contiene ningún mandato o se ejecuta el mandato en background, se volverá a
  mostrar el prompt a la espera de una nueva línea.
  * Si alguno de los mandatos a ejecutar no existe, el programa debe mostrar el error “mandato: No se encuentra
  el mandato”.
  * Si se produce algún error al abrir cualquiera de los ficheros de las redirecciones, debe mostrarse el error
  “fichero: Error. Descripción del error”.
  * Ni el minishell ni los procesos en background deben finalizar al recibir las señales desde teclado SIGINT (CtrlC)
  y SIGQUIT (Ctrl-\) mientras que los procesos que se lancen deben actuar ante ellas, manteniendo la acción
  por defecto. 
  Objetivos parciales
  * Ser capaz de reconocer y ejecutar en foreground líneas con un solo mandato y 0 o más argumentos. (0.5
  puntos)
  * Ser capaz de reconocer y ejecutar en foreground líneas con un solo mandato y 0 o más argumentos, redirección
  de entrada estándar desde archivo y redirección de salida a archivo. (1 punto)
  * Ser capaz de reconocer y ejecutar en foreground líneas con dos mandatos con sus respectivos argumentos,
  enlazados con ‘|’, y posible redirección de entrada estándar desde archivo y redirección de salida a archivo. (1
  punto)
  * Ser capaz de reconocer y ejecutar en foreground líneas con más de dos mandatos con sus respectivos
  argumentos, enlazados con ‘|’, redirección de entrada estándar desde archivo y redirección de salida a archivo.
  (4 puntos)
  * Ser capaz de ejecutar el mandato cd (1 punto). Mientras que la mayoría de los mandatos son programas del
  sistema, cd es un mandato interno que debe ofrecer el propio intérprete de mandatos. El mandato cd debe
  permitir tanto el acceso a través de rutas absolutas como relativas, además de la posibilidad de acceder al
  directorio especificado en la variable HOME si no recibe ningún argumento, escribiendo la ruta absoluta del
  nuevo directorio actual de trabajo. Para el correcto cambio de directorio el comando cd se debe ejecutar sin
  pipes.
  * Ser capaz de reconocer y ejecutar tanto en foreground como en background líneas con más de dos mandatos
  con sus respectivos argumentos, enlazados con ‘|’, redirección de entrada estándar desde archivo y
  redirección de salida a archivo. Para su correcta demostración, se deben realizar los mandatos internos jobs y
  fg (1.5 puntos):
  o jobs: muestra la lista de procesos que se están ejecutando en segundo plano en la minishell (no es
  necesario considerar aquellos procesos pausados con Ctrl-Z). El formato de salida será similar al del
  mandato jobs del sistema:
  [1]+ Running find / -name hola | grep h &
  o fg: reanuda la ejecución del proceso en background identificado por el número obtenido en el mandato
  jobs, indicando el mandato que se está ejecutando. Reorienta su entrada estándar y salidas estándar a la
  terminal de usuario. Si no se le pasa ningún identificador se pasa a foreground el último mandato en
  background introducido.
  * Evitar que los comandos lanzados en background y el minishell mueran al enviar las señales desde el teclado
  SIGINT y SIGQUIT, mientras los procesos en foreground respondan ante ellas. (1 punto)
  Nota: las puntuaciones para cada objetivo parcial son las puntuaciones máximas que se pueden obtener si se cumplen
  esos objetivos.
  Nota: no se debe hacer un programa separado para cada objetivo, sino un único programa genérico que cumpla con
  todos los objetivos simultáneamente.
  
## Requisitos // Requirements 
<b>[ESP]</b>Estos comandos están diseñados para ejecutarse en un sistema operativo UNIX (Con compilador GCC). <b> Cualquier distribucion de Mac OSX o Linux </b> 

<b>[EN]</b>These commands are designed to work in a UNIX (GCC Compiler needed). <b>Max OSX or Any Linux distribution</b>
