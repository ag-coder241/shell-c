#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include "executor.h"
#include "builtins.h"
#include "redirection.h"
#include <stdlib.h>
#include <string.h>
#include "parser.h"

void execute(char **args){

    if(args[0] == NULL) return;

    // check for pipe
    char **commands[16];
    int num_cmds = split_pipelines(args, commands, 16);

    if (num_cmds == -1) {
        fprintf(stderr, "syntax error: expected command after |\n");
        return;
    }

    if(num_cmds > 1){
        // multipipeline logic
        int pipefd[2];
        int prev_pipe_read = -1; // keep track of the input for the next command
        pid_t pids[16];

        for(int i = 0; i < num_cmds; i++){
            // for n commands we only need n-1 commands

            // for last command we don't need the pipe
            if(i < num_cmds-1){
                if(pipe(pipefd) == -1){
                    perror("pipefd");
                    return;
                }
            }

            pid_t pid = fork();

            if(pid == 0){
                // child process

                // setup input
                // if there was a previous pipe, read from it
                if(prev_pipe_read != -1){
                    dup2(prev_pipe_read, STDIN_FILENO);
                    close(prev_pipe_read);
                }

                // setup output
                // if not the last command, write to the pipe read
                if(i < num_cmds-1){
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);
                }

                // execute
                if (is_builtin(commands[i][0])) {
                    run_builtin(commands[i]);
                    exit(0);
                } else {
                    execvp(commands[i][0], commands[i]);
                    perror("execvp");
                    exit(1);
                }
            }
            else if(pid < 0){
                perror("fork");
                return;
            }

            // parent process
            pids[i] = pid; // save PID to wait later

            // clean previous read
            if(prev_pipe_read != -1){
                close(prev_pipe_read);
            }

            // prepare next read
            if(i < num_cmds-1){
                prev_pipe_read = pipefd[0];
                close(pipefd[1]); // parent doesn't uses WRITE end
            }
        }

            // wait for all children
        for(int i = 0; i < num_cmds; i++){
            waitpid(pids[i], NULL, 0);
        }
        return;
    }

    // current file descriptors
    int saved_stdout = dup(STDOUT_FILENO);
    int saved_stderr = dup(STDERR_FILENO);

    // always apply redirections first
    if (has_redirection(args)) {
        if (handle_redirection(args) != 0) {
            /* restore fds on error */
            dup2(saved_stdout, STDOUT_FILENO);
            dup2(saved_stderr, STDERR_FILENO);
            close(saved_stdout);
            close(saved_stderr);
            return;
        }
    }

    if(is_builtin(args[0])){
        // builtins run in the terminal itself
        // no forking here
        run_builtin(args);
        
        // restore fds
        dup2(saved_stdout, STDOUT_FILENO);
        dup2(saved_stderr, STDERR_FILENO);
        close(saved_stdout);
        close(saved_stderr);
        return;
    }

    pid_t pid = fork();

    if(pid == 0){
        // child process
        execvp(args[0], args);
        printf("%s: command not found\n", args[0]);
        exit(127);

    }
    else if(pid > 0){
        // parent process wait
        wait(NULL);
    }
    else{
        perror("fork");
    }

    // restore fds
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stdout);
    close(saved_stderr);
}