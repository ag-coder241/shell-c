#include "parser.h"
#include <string.h> 
#include <stddef.h> 

int find_pipe(char **args){
  for(int i = 0; args[i] != NULL ; i++){
    if(strcmp(args[i], "|") == 0){
      return i;
    }
  }
  return -1;
}

int split_pipelines(char **args, char ***cmds, int max_cmds){
  int count = 0;
  cmds[count++] = args; // first command always start at args[0]

  for(int i = 0; args[i] != NULL; i++){
    if(strcmp(args[i], "|") == 0){
      args[i] = NULL;

      // next command start at i+1
      if(args[i+1] != NULL){
        cmds[count++] = &args[i+1];
        if(count >= max_cmds) break;
      }
      else{
        return -1;
      }
    }
  }
  return count;

}

void parser(char *command, char **args)
{
  int argc = 0;
  int in_single = 0;
  int in_double = 0;
  char *start = command;
  /// inside quotes everything is literal

  for (char *p = command;; p++)
  {

    if (*p == '\\' && !in_single && !in_double)
    {
      memmove(p, p + 1, strlen(p)); // remove backslash
      continue;                     // keep escaped character
    }

    if (*p == '\\' && in_double)
    {
      // backslash acts special
      if (*(p + 1) == '"' || *(p + 1) == '\\')
      {
        memmove(p, p + 1, strlen(p));
        continue;
      }
    }

    if (*p == '\'' && !in_double)
    {
      // if inside single quotes
      in_single = !in_single;
      memmove(p, p + 1, strlen(p)); // remove quote
      p--;                          // stay at same index
      continue;
    }
    if (*p == '\"' && !in_single)
    {
      // if inside double quotes
      in_double = !in_double;
      memmove(p, p + 1, strlen(p)); // remove quote
      p--;                          // stay at same index
      continue;
    }

    if ((*p == ' ' && !in_single && !in_double) || *p == '\0')
    {
      // if we see a space outside quotes or reached the end we have to split
      if (start != p)
      {
        args[argc++] = start;
      }

      if (*p == '\0')
        break;
      *p = '\0';
      start = p + 1; // move to next argument
    }
  }
  args[argc] = NULL;
}

