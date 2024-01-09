#ifndef EXEC_H
#define EXEC_H

#include "defs.h"
#include "types.h"
#include "utils.h"
#include "freecmd.h"

extern struct cmd *parsed_pipe;

void exec_cmd(struct cmd *c);
char **parse_args(struct cmd *cmd);

#endif  // EXEC_H
