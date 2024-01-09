#include "exec.h"
#include "printstatus.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
	for (int i = 0; i < eargc; i++) {
		int eqIdx = block_contains(eargv[i], '=');

		if (eqIdx == -1) {
			continue;
		}

		char key[eqIdx + 1];
		get_environ_key(eargv[i], key);

		char value[strlen(eargv[i]) - eqIdx];
		get_environ_value(eargv[i], value, eqIdx);

		if (setenv(key, value, 1) == -1) {
			printf_debug("Error seteando variables entorno: %s\n",
			             strerror(errno));
			exit(1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	if (flags & O_CREAT) {
		return open(file, flags, S_IRUSR | S_IWUSR);
	}
	return open(file, flags);
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		//
		// Your code here
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);

		int status = execvp(e->argv[0], e->argv);

		if (status == -1) {
			printf_debug("case BACK: execvp failed\n");
			if (cmd)
				free_command(cmd);
			exit(EXIT_FAILURE);
		}

		free_command(cmd);
		free(e);

		break;
	}
	case BACK: {
		// runs a command in background
		//
		// Your code here
		b = (struct backcmd *) cmd;
		e = (struct execcmd *) b->c;

		set_environ_vars(e->eargv, e->eargc);

		setpgid(0, getpgid(getppid()));
		// printf("get process group id:  %d\n", getpgid(0));
		// printf("get parentr process group id:  %d\n", getppid());
		int status = execvp(e->argv[0], e->argv);
		if (status == -1) {
			printf_debug("case BACK: execvp failed\n");
			if (cmd)
				free_command(cmd);
			exit(EXIT_FAILURE);
		}
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd *) cmd;

		int fileInFD = 0;
		int fileOutFD = 0;
		int fileErrFD = 0;

		if (strlen(r->in_file) > 0) {
			fileInFD =
			        open_redir_fd(r->in_file, O_CLOEXEC | O_RDONLY);

			dup2(fileInFD, 0);
		}
		if (strlen(r->out_file) > 0) {
			if (r->out_file[0] == '>') {
				fileOutFD = open_redir_fd(
				        r->out_file + 1,
				        O_CREAT | O_TRUNC | O_RDWR | O_CLOEXEC);
			} else {
				fileOutFD = open_redir_fd(
				        r->out_file,
				        O_CREAT | O_TRUNC | O_RDWR | O_CLOEXEC);
			}

			dup2(fileOutFD, 1);
		}
		if (strlen(r->err_file) > 0) {
			if (r->err_file[0] != '&') {
				fileErrFD = open_redir_fd(
				        r->err_file,
				        O_CREAT | O_TRUNC | O_RDWR | O_CLOEXEC);

				dup2(fileErrFD, 2);

			} else {
				dup2(1, 2);
			}
		}

		int status = 0;
		if (fileInFD > 0 || fileOutFD > 0 || fileErrFD > 0) {
			status = execvp(r->argv[0], r->argv);
		}

		if (fileInFD > 0) {
			close(fileInFD);
			close(0);
		}
		if (fileOutFD > 0) {
			close(fileOutFD);
			close(1);
		}
		if (fileErrFD > 0) {
			close(fileErrFD);
			close(2);
		}
		if (cmd) {
			free_command(cmd);
		} else {
			free(r);
		}
		if (status == -1) {
			printf_debug("case REDIR: execvp failed: %s\n",
			             strerror(errno));
			exit(EXIT_FAILURE);
		} else {
			exit(EXIT_SUCCESS);
		}

		break;
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here
		p = (struct pipecmd *) cmd;
		pid_t fork_pid = fork();
		if (fork_pid > 0) {
			wait(&status);
		} else if (fork_pid == 0) {
			int pipe_fd[2];
			if (pipe(pipe_fd) < 0) {
				printf_debug("Error ejecutando pipe: %s\n",
				             strerror(errno));
				exit(EXIT_FAILURE);
			}
			pid_t fork_left_pid = fork();
			pid_t fork_right_pid = fork();
			if (fork_left_pid == 0) {
				free_command(p->rightcmd);
				free(p);
				close(pipe_fd[0]);
				dup2(pipe_fd[1], 1);
				close(pipe_fd[1]);
				exec_cmd(p->leftcmd);
			} else if (fork_left_pid < 0) {
				printf_debug(
				        "Error ejecutando fork izquierdo: %s\n",
				        strerror(errno));
				close(pipe_fd[0]);
				close(pipe_fd[1]);
				exit(EXIT_FAILURE);
			}
			if (fork_right_pid > 0) {
				if (fork_left_pid > 0) {
					close(pipe_fd[1]);
					close(pipe_fd[0]);
					waitpid(fork_left_pid, NULL, 0);
					waitpid(fork_right_pid, &status, 0);
				}
			} else if (fork_right_pid == 0) {
				if (fork_left_pid > 0) {
					free_command(p->leftcmd);
					free(p);
					close(pipe_fd[1]);
					dup2(pipe_fd[0], 0);
					close(pipe_fd[0]);
					exec_cmd(p->rightcmd);
				}
			} else {
				printf_debug(
				        "Error ejecutando fork derecho: %s\n",
				        strerror(errno));
				close(pipe_fd[0]);
				close(pipe_fd[1]);
			}
		} else {
			printf_debug("Error ejecutando fork: %s\n",
			             strerror(errno));
		}
		// free the memory allocated
		// for the pipe tree structure
		// free_command(parsed_pipe);
		free_command(cmd);
		if (WEXITSTATUS(status)) {
			exit(WEXITSTATUS(status));
		} else {
			exit(status);
		}

		break;
	}
	}
}
