#include "builtin.h"

int
indexOf(char *buf, char *subStr)
{
	int position = -1;

	if (buf && subStr) {
		char *result = strstr(buf, subStr);
		if (result) {
			position = result - buf;
		}
	}

	return position;
}

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	// Your code here
	if (indexOf(cmd, "exit") == 0) {
		exit(EXIT_SUCCESS);
		return 1;
	}

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	// Your code here
	if (indexOf(cmd, "cd") == 0) {
		char *chgDir;

		if (strlen(cmd) <= 3) {
			chgDir = getenv("HOME");
		} else {
			chgDir = cmd + 3;
		}

		if (chdir(chgDir) == -1) {
			printf_debug("Error cambiando directorio: %s\n",
			             strerror(errno));
			return 0;
		}

		// update the 'prompt' with the new directory
		char *directory = (char *) malloc(sizeof(char) * 1024);
		char *currDir = getcwd(directory, 1024);

		if (currDir == NULL) {
			printf_debug("Error obteniendo directorio actual: %s\n",
			             strerror(errno));
			return 0;
		}

		memset(prompt, 0, sizeof prompt);
		memcpy(prompt, currDir, strlen(currDir));
		free(currDir);

		return 1;
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	// Your code here
	if (indexOf(cmd, "pwd") == 0) {
		char *dir = (char *) malloc(sizeof(char) * 1024);

		char *currDir = getcwd(dir, 1024);

		if (currDir == NULL) {
			printf_debug("Error obteniendo directorio actual: %s\n",
			             strerror(errno));
			return 0;
		}

		printf("%s\n", currDir);

		return 1;
	}

	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
