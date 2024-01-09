# sched

### Parte 1.1, de modo kernel a modo usuario
![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/f5abbe7a-971a-4eb5-bcff-f11e69c3cb2f)

En la imagen se ve el estado del stack antes de hacer pop. En las capturas siguientes se ve el estado de los registros a medida que se van popeando los campos.

![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/45007590-145d-44af-a413-0d04cf4f4995)

En la imagen se ve como se escriben los registros de pushregs, %es y %ds

![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/259fe4f5-d904-46e8-a22d-6585e26e3e7d)

En la ultima imagen, se ve como se escriben los registros %esp, %eip, %cs, %eflags.

### Parte 1.2, de modo usuario a modo kernel
![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/e89b612b-cee7-4a22-9f0a-2783cf9ab5e7)

En esta imagen se ve como se guarda el Trapframe en memoria, con los campos que estan en los registros

### Parte 3, aclaracion de los procesos de usuario agregados para probar el scheduler con prioridades

#### Prioridad del proceso hijo
Se establece la prioridad del proceso hijo como la misma que la del padre. Esto se puede verificar corriendo el programa de usuario priorityfork.c, mediante ENV_CREATE(user_priorityfork, ENV_TYPE_USER); en init.c

La salida de dicho programa es:
![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/8697ac2c-de47-4e54-9fe1-53b7daeb9301)

#### Seguridad de la syscalls de cambiar prioridad
Se testea la seguridad, tal que un proceso no pueda cambiar su prioridad a un valor numerico menor que la que tiene en un momento dado. Esto se puede verificar corriendo el programa de usuario prioritysecure.c, mediante ENV_CREATE(user_prioritysecure, ENV_TYPE_USER); en init.c

La salida de dicho programa es:
![image](https://github.com/fiubatps/sisop_2023b_g33/assets/53508927/41cf8423-34e2-445f-b572-1050a4172157)

#### Estadsticas de scheduling
Se hizo un programa de usuario que juega con distintas prioridades, priorityfork2.c (user_priorityfork2).

La salida de dicho programa es:

priority 0 executions: 155

priority 1 executions: 116

priority 2 executions: 114

priority 3 executions: 110

Estos valores cambian segun la cantidad de tickets asignados a cada proceso, y segun la cantidad de ciclos que se requieren para el reseteo de prioridad. Dichos valores estan definidos en las macros /inc/env.h TICKET_AMMOUNT, y en /kern/sched.c CALLS_TO_RESET_PRIORITY
