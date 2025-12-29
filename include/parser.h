#ifndef PARSER_H
#define PARSER_H

void parser(char *command, char **args);
int find_pipe(char **args);
int split_pipelines(char **args, char ***cmds, int max_cmds);

#endif
