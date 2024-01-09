#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#define CALLS_TO_RESET_PRIORITY 40
int sched_calls = 1;
long executions_per_priority[LOWEST_PRIORITY-HIGHEST_PRIORITY+1] = {0};
void sched_halt(void);
void reset_priorities_and_tickets(void);

void
sched_yield(void){
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	envid_t idx_envs = 0;
	envid_t idx_curenv = 0;
	struct Env* env_to_run = NULL;
	if (curenv != NULL) {
		idx_envs = ENVX(curenv->env_id) + 1;
		idx_curenv = curenv->env_id;
		env_to_run = curenv;
	}
	
	#ifdef RND_ROB
	for(idx_envs; idx_envs<NENV; idx_envs++){
		if(envs[idx_envs % NENV].env_status == ENV_RUNNABLE){
			env_to_run = &envs[idx_envs % NENV];
			env_run(env_to_run);
		}
	}

	for(idx_envs=0;idx_envs < idx_curenv; idx_envs++){
		if(envs[idx_envs % NENV].env_status == ENV_RUNNABLE){
			env_to_run = &envs[idx_envs % NENV];
			env_run(env_to_run);
		}
	}
	#endif

	#ifdef SCHED_PRIOR
	
	sched_calls++;
	
	
	if (sched_calls % CALLS_TO_RESET_PRIORITY == 0){
		reset_priorities_and_tickets();
	}
	int cur_priority = HIGHEST_PRIORITY;

	while (cur_priority <= LOWEST_PRIORITY) {
		for(idx_envs; idx_envs<NENV; idx_envs++){
			struct Env* env = &envs[idx_envs % NENV];
			
			if(env->env_status == ENV_RUNNABLE && env->env_priority == cur_priority){
				if(cur_priority==LOWEST_PRIORITY){
					executions_per_priority[cur_priority-HIGHEST_PRIORITY]++;
					env_run(env);
				}
				else if(env->env_tickets > 0){
					env->env_tickets--;
					executions_per_priority[cur_priority-HIGHEST_PRIORITY]++;
					env_run(env);
				}
				else if (env->env_tickets == 0){
					env->env_priority++;
					executions_per_priority[cur_priority-HIGHEST_PRIORITY]++;
					env->env_tickets = TICKET_AMMOUNT;
				}
			}
		}
		for(idx_envs=0;idx_envs < idx_curenv; idx_envs++){
			struct Env* env = &envs[idx_envs % NENV];
			
			if(env->env_status == ENV_RUNNABLE && env->env_priority == cur_priority){
				if(cur_priority==LOWEST_PRIORITY){
					executions_per_priority[cur_priority-HIGHEST_PRIORITY]++;
					env_run(env);
				}
				else if(env->env_tickets > 0){
					executions_per_priority[cur_priority-HIGHEST_PRIORITY]++;
					env->env_tickets--;
					env_run(env);
				}
				else if (env->env_tickets == 0){
					executions_per_priority[cur_priority-HIGHEST_PRIORITY]++;
					env->env_priority++;
					env->env_tickets = TICKET_AMMOUNT;
				}
			}
		}
		cur_priority++;
	}
	#endif

	if (curenv != NULL && curenv->env_status== ENV_RUNNING){
		env_run(curenv);
	}
	// Halt this CPU when there is nothing to do. Wait until the
	sched_halt();
	// timer interrupt wakes it up. This function never returns.	
}



void reset_priorities_and_tickets(void){
	for(envid_t idx_envs=0; idx_envs<NENV; idx_envs++){
		envs[idx_envs % NENV].env_priority = HIGHEST_PRIORITY;
		envs[idx_envs % NENV].env_tickets = TICKET_AMMOUNT;
	}
}


void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		cprintf("sched calls: %d\n",sched_calls);
		for(int i=0; i<=LOWEST_PRIORITY - HIGHEST_PRIORITY; i++){
			cprintf("priority %d executions: %d\n",i+HIGHEST_PRIORITY,executions_per_priority[i]);
		}
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
