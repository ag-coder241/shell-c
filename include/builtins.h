#ifndef BUILTINS_H
#define BUILTINS_H

int is_builtin(const char *cmd);
int run_builtin(char **args);

void load_history(void);
void save_history_cleanup(void);

#endif
