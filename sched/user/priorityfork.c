#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int id;
    sys_set_priority(sys_get_priority()+1);
	if ((id = fork()) < 0)
		panic("fork: %e", id);

	if (id == 0){
        cprintf("prioridad proceso hijo = %d\n", sys_get_priority());
    }
	else {
        cprintf("prioridad proceso padre = %d\n", sys_get_priority());
    }

}