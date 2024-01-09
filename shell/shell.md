# shell

### Búsqueda en $PATH

---
 	¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?
 		
 		La syscall execve(2) toma como parámetro de entrada un archivo que desea ejecutar, 
	con el cual sobrescribe el espacio de memoria del proceso actual (heap, stack y 
	data), manteniendo su PID.
		La familia de wrappers exec(3) reemplaza la imagen de proceso actual con una nueva 
	imagen de proceso, generando un nuevo PID. Esta familia exec(3) es un conjunto de 
	funciones escritas encima de execve(2), pero con ligeros cambios en su 
	comportamiento segun su nombre (exec + *):
		-l: recibe una lista de argumentos para usar como argumentos de linea 
		de comando para el programa a ejecutar
		-v: recibe un vector con los argumentos a usar en el nuevo programa
		-e: hay que especificar el entorno del nuevo programa
		-p: simula la forma de buscar el archivo a ejecutar como lo hace la shell
			usando la variable PATH si no se usa "/" para especificar el nombre
			del archivo a ejecutar


 	¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

 		Si, ya que, al estar basado en el mismo funcionamiento y principios que 
	execve(2), presenta sus mismos escenarios de fallas.
	Por esa razón, para la mayoria de los casos cuando exec(3) falla, se retorna
	al proceso principal de la shell para que esta trate el error. Pero, para otros
	casos (principalmente aquellos en los que se agotan recursos) es imposible
	retornar, resultando que se mate al proceso original (que en este caso seria
	el hijo que crea run_cmd()).


### Procesos en segundo plano

---
	Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

		A la hora de hacerse el fork() para que el Hijo se encargue del proceso en
	segundo plano, lo que se hace del lado del Padre es usar waitpid() con el 
	flag WNOHANG, que lo que hace es no bloquear al Padre si el proceso
	secundario todavia no se completo. De esta manera, el proceso en segundo
	plano puede continuar cada vez que el Padre esta en pausa.


### Flujo estándar

---
	Investigar el significado de 2>&1, explicar cómo funciona su forma general
	Mostrar qué sucede con la salida de cat out.txt en el ejemplo.
	Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

	$ ls -C /home /noexiste >out.txt 2>&1

### Tuberías múltiples

---
	Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
	¿Cambia en algo?
	¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

	Al ejecutar con un pipe, el exit code no se imprime como lo haría con la ejecución de un programa normal.
	Si alguno de los comandos falla (no importa de qué lado del pipe este), se sale inmediatamente y se imprime el mensaje del comando que fallo nada más.


### Variables de entorno temporarias

---
	¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

		Las variables de entornos temporales, son variables locales del proceso, o sea, cuando 
	el proceso termina, las variables temporales dejan de existir. Por lo tanto, es necesario 
	hacerlo luego del fork, ya que el fork genera 2 procesos distintos en los que tienen que 
	existir dichas variables.


	En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
	¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
	Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

		El comportamiento va a ser el mismo si sólo se desean usar esas variables, ya que para cuando no 
	pasamos esas variables como argumento, exec(3) busca variables de entorno ya seteadas (internas del 
	kernel o aquellas que se hayan creado con setenv(3)).
		La única diferencia es que, para aquellas funciones que se pueden pasar variables de entorno por 
	argumento, entonces, su entorno solo va a consistir de esas variables (no puede usar variables de un
	entorno externo). En ese caso, el comportamiento va a ser distinto.
		Lo que podríamos hacer es agregar en el array de variables pasadas por argumento la variable externa “environ” 
	(que son las variables de entorno que usan el resto de las funciones exec(3)) junto con el resto de variables 
	que deseemos usar.


### Pseudo-variables

---
	Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).

	-$? : devuelve el exit code del último comando ejecutado en foreground.
	-$! : devuelve el exit code del último comando ejecutado en background.
	-$_ : devuelve el último argumento del último comando ejecutado.
	-$$ : devuelve el PID del shell actual.


### Comandos built-in

---
	¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

		El pwd es el que se puede implementar sin necesidad de ser built-in. (la shell lo implementa como built-in, pero 
	el bash lo implementa como programa, existen en el /bin).
		El built-in es mucho más rápido que un programa, ya que en la ejecución de un programa se requiere de las llamadas a las syscalls.


### Segundo plano avanzado

---
	¿Por qué es necesario el uso de señales?

 	Las señales son un tipo de interrupcion que nos permiten detectar el surgimiento de un evento y poder responder a este de forma asincrónca. Sin hacer uso de señales, dichos eventos podrían ser tratados de forma periódica, pero la utilización de señales y syscalls como sigaction nos permiten realizar dichas acciones en el momento.
  	En este caso, las señales se usan para la liberación de recursos que podrían ser necesarios para la ejecución de otros procesos. Liberar esos recursos de forma inmediata es importante dado que permite que los otros procesos, ahora mismo en ejecución, puedan disponer de ellos
	
	
### Segundo plano avanzado: explicacion paso a paso

#### Seteo PGID convenientemente:


	Es necesario que el PGID del proceso en segundo plano sea igual al PGID del padre del proceso del segundo plano, por lo que en exec.c > exec_cmd(..) en case: BACK, se configura el PGID del proceso acordemente.

	setpgid(0, getpgid(getppid()));

	Esta necesidad deriva de que se puede usar waitpid con pid=0, tal que solo espera por procesos hijo que tengan el mismo pgid que el padre.

#### Tratamiento especial para procesos en segundo plano;

	En runcmd.c > run_cmd(), el proceso padre después del fork hace un waitpid bloqueante del proceso hijo, pero esto no es el funcionamiento que debe tener un proceso en segundo plano. Por lo tanto, se hace un if donde se verifica si el proceso creado es de tipo BACK, en caso de que no, se hace dicho wait bloqueante, pero en caso de que si, se imprime información acerca del proceso.

#### Signal action, stack alternativo
	En sh.c se configura mediante la funcion sigaction(), el handler correspondiente a la señal SIGCHILD.
 	En set_signal_action() se configura el stack alternativo, y su puntero se guarda en una estructura de
  	tipo sigaction, la cual pasada como parámetro a la función sigaction(). En dicho struct sigaction, se
   	debe configurar el flag SA_RESTART para reiniciar cualquier syscall que haya sido interrumpida por
    	efecto de la señal. También se debe guardar en dicho struct una referencia al handler que haya sido
     	implementado.

 #### Sigchld handler
 	En dicho handler se hace un waitpid no bloqueante con el parámetro PID = 0, tal que, como fue explicado
  	anteriormente, solo se haga wait a los procesos hijos cuyo PGID sean iguales al del proceso invocante
   	del wait. En caso de que un proceso en segundo plano finalizado haya sido interceptado por el waitpid,
    	esta devuelve su PID. En caso de que devuelva 0, significa que contiene procesos en segundo plano con
     	PGID compatible pero que aún están corriendo. Y en caso de que devuelva -1, depende el valor de errno.
      	Si errno = ECHILD, significa que la señal SIGCHLD fue emitida por un proceso que no cumple con el
       	requisito de PGID, lo cual es propio de la finalización de los procesos que no son en segundo plano
	y no fue el pgid configurado acordemente.

