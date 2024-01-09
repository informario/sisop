#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };
void
sigchld_handler(int signal)
{
	if (signal == SIGCHLD) {
		int childpid = waitpid(0, NULL, WNOHANG);
		char buf[100];

		if (childpid > 0) {
			int len = snprintf(
			        buf, 100, "==> terminado: PID=%d\n", childpid);
			write(STDOUT_FILENO, buf, len);
		} else if (childpid == 0) {
			// printf("Child process still running\n") ;
		} else {
			if (errno == ECHILD) {
				// printf(" Error ECHILD!!\n") ;
			} else if (errno == EINTR) {
				// printf_debug(" Error EINTR!!\n") ;
				int len = snprintf(
				        buf, 100, " Error EINTR!!\n", childpid);
				write(STDOUT_FILENO, buf, len);
			} else {
				// printf_debug("Error EINVAL!!\n") ;
				int len = snprintf(
				        buf, 100, " Error EINVAL!!\n", childpid);
				write(STDOUT_FILENO, buf, len);
			}
		}
	}
}

void
set_signal_action(void)
{
	stack_t ss;

	ss.ss_sp = malloc(SIGSTKSZ);
	if (ss.ss_sp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1) {
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &sa, NULL);
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	set_signal_action();
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();
	run_shell();

	return 0;
}
