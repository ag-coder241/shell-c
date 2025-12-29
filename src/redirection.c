#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <fcntl.h>      
#include <sys/wait.h>   
#include <string.h>     
#include <errno.h>    

#include "redirection.h"

int has_redirection(char **args){
  int flag = 0;
  for (int i = 0; args[i]; i++) {
    if (!strcmp(args[i], ">")  || !strcmp(args[i], ">>") ||
        !strcmp(args[i], "1>") || !strcmp(args[i], "1>>") ||
        !strcmp(args[i], "2>") || !strcmp(args[i], "2>>")) {
        flag = 1;
        break;
    }
}

return flag;
}

int handle_redirection(char **args){
    int redirect_index = -1;
    int err_redirect = -1;
    int out_append = -1;
    int err_append = -1;
    
    // check if the command is for redirection
    for (int i = 0; args[i] != NULL; i++)
    {
      if (strcmp(args[i], ">") == 0 || strcmp(args[i], "1>") == 0)
      {
        redirect_index = i;
        break;
      }
      if (strcmp(args[i], "2>") == 0)
      {
        err_redirect = i;
        break;
      }
      if (strcmp(args[i], ">>") == 0 || strcmp(args[i], "1>>") == 0)
      {
        out_append = i;
        break;
      }
      if (strcmp(args[i], "2>>") == 0)
      {
        err_append = i;
        break;
      }
    }

    if (redirect_index != -1 && args[redirect_index + 1] == NULL)
    {
      printf("syntax error: expected file after >\n");
      return 1;
    }
    if (err_redirect != -1 && args[err_redirect + 1] == NULL)
    {
      printf("syntax error: expected file after >\n");
      return 1;
    }
    if (out_append != -1 && args[out_append + 1] == NULL)
    {
      printf("syntax error: expected file after >\n");
      return 1;
    }
    if (err_append != -1 && args[err_append + 1] == NULL)
    {
      printf("syntax error: expected file after >\n");
      return 1;
    }

      int out_fd = -1; // fd for command out redirection
      int err_fd = -1; // fd for error redirection

      if (redirect_index != -1)
      {
        out_fd = open(args[redirect_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        args[redirect_index] = NULL; // make the > symbol null
        args[redirect_index+1] = NULL; // make the file name also null
      }

      if (err_redirect != -1)
      {
        err_fd = open(args[err_redirect + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        args[err_redirect] = NULL;
        args[err_redirect+1] = NULL;
      }

      if (out_append != -1)
      {
        out_fd = open(args[out_append + 1],
                      O_WRONLY | O_CREAT | O_APPEND, 0644);
        args[out_append] = NULL;
        args[out_append+1] = NULL;
      }

      if (err_append != -1)
      {
        err_fd = open(args[err_append + 1],
                      O_WRONLY | O_CREAT | O_APPEND, 0644);
        args[err_append] = NULL;
        args[err_append+1] = NULL;
      }

      if (out_fd != -1) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
      }

      if (err_fd != -1) {
        dup2(err_fd, STDERR_FILENO);
        close(err_fd);
      }

      return 0;
    }