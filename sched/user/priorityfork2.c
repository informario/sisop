#include <inc/lib.h>
///Requiere 4 prioridades (i.e 0 a 3)
void loop_print(int param);
void
umain(int argc, char **argv)
{
	int id;
	if ((id = fork()) < 0)
		panic("fork: %e", id);
	if (id == 0){
        int id;
        if ((id = fork()) < 0)
            panic("fork: %e", id);
        if (id == 0){
            int id;
            if ((id = fork()) < 0)
                panic("fork: %e", id);
            if (id == 0){
                int id;
                if ((id = fork()) < 0)
                    panic("fork: %e", id);
                if (id == 0){
                    //Quinto
                }
                else {
                    //Cuarto
                    sys_set_priority(LOWEST_PRIORITY-3);
                    loop_print(0);
                }
            }
            else {
                //Tercero
                sys_set_priority(LOWEST_PRIORITY-2);
                loop_print(1);
            }
        }
        else {
            //Segundo
            sys_set_priority(LOWEST_PRIORITY-1);
            loop_print(2);
        }
    }
	else {
        //Primero
        sys_set_priority(LOWEST_PRIORITY);
        loop_print(3);
    }
}

//  sys_set_priority(sys_get_priority()+1);

//cprintf("prioridad proceso hijo = %d\n", sys_get_priority());

// cprintf("prioridad proceso padre = %d\n", sys_get_priority());
void loop_print(int param){
    int i = 0;
    while(true){
        for(int x=0; x<10000; x++){
            if(sys_get_priority()<param){
                sys_set_priority(param);
            }
        }
        i++;
        cprintf("%d", param);
        if(i>=10){
            exit();
        }
    }
}