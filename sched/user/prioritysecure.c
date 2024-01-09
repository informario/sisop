// hello, world
#include <inc/lib.h>
void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	uint32_t priority = sys_get_priority();
	sys_set_priority(priority+2);
	priority = sys_get_priority();
	cprintf("my priority is %d\n", priority);
	
	uint32_t return_code = sys_set_priority(priority-1);
	cprintf("after setting a forbidden priority value (lesser by 1 than previous), function returns error code 3: %d\n", return_code);
	
	priority = sys_get_priority();
	return_code = sys_set_priority(priority);
	cprintf("in this case, after setting a valid priority, error code should be 0 because there is no error: %d\n", return_code);

}

