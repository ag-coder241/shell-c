#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "builtins.h"
#include <sys/stat.h>
#include <readline/history.h>

static int history_file_index = 0; // to keep track of how many lines written to a file
static int history_default_file_index = 0; // to keep track of how many lines files saved to default history file

void get_history_path(char *buffer, size_t size){
    char *histfile = getenv("HISTFILE");
    if(histfile){
        snprintf(buffer, size, "%s", histfile);
    }
    else{
        char *home = getenv("HOME");
        if (home == NULL) {
            snprintf(buffer, size, ".shell_history");
        } else {
            snprintf(buffer, size, "%s/.shell_history", home);
        }
    }
}

int append_history_to_file(int new_entries, const char *path) {
    if (new_entries <= 0) return 0;

    FILE *fp = fopen(path, "a"); //
    if (!fp) return -1;

    // Determine where to start reading from memory
    // history_base is the logical offset provided by readline
    int start_index = history_base + history_length - new_entries;

    for (int i = 0; i < new_entries; i++) {
        // Fetch the entry at the calculated logical index
        HIST_ENTRY *entry = history_get(start_index + i);
        if (entry) {
            fprintf(fp, "%s\n", entry->line);
        }
    }

    fclose(fp);
    return 0;
}

void load_history(void){
    char path[1024];
    get_history_path(path, sizeof(path));

    if(access(path, F_OK) == 0){
        read_history(path);
    }

    history_default_file_index = history_length; // set to current length so that we don't re append this again
}

void save_history_cleanup(void) {
    char path[1024];
    get_history_path(path, sizeof(path));

    // Calculate how many new commands were executed in this session
    int new_cmds = history_length - history_default_file_index;

    if (new_cmds > 0) {
        // Append only the new commands to the file
        if (append_history_to_file(new_cmds, path) == 0) {
            // printf("History appended to %s\n", path);
        } else {
            // perror("Failed to append history");
        }
    }
}

int is_builtin(const char *cmd)
{
    return !strcmp(cmd, "echo") ||
           !strcmp(cmd, "cd") ||
           !strcmp(cmd, "pwd") ||
           !strcmp(cmd, "type") ||
           !strcmp(cmd, "history") ||
           !strcmp(cmd, "exit");
}

int run_builtin(char **args)
{
    // exit
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    
    // clear
    if (strcmp(args[0], "clear") == 0)
    {
        // \033[H  -> Move cursor to Home position (Top Left)
        // \033[2J -> Clear the entire screen
        printf("\033[H\033[2J");
        return 1;
    }

    // history
    if (strcmp(args[0], "history") == 0) {
        char path[1024];
        
        // file handling
        if (args[1] != NULL && args[1][0] == '-') {
            char *flag = args[1];
            char *target_file = args[2];

            // Determine target file
            if (target_file) {
                snprintf(path, sizeof(path), "%s", target_file);
            } else {
                get_history_path(path, sizeof(path));
            }

            // Case 1: Append (-a)
            if (strcmp(flag, "-a") == 0) {
                int new_cmds = history_length - history_file_index;
                if (new_cmds > 0) {
                    if (append_history_to_file(new_cmds, path) != 0) {
                        perror("history");
                    } else {
                        // Only update index if we appended to the MAIN history file
                        char default_path[1024];
                        get_history_path(default_path, sizeof(default_path));
                        if (strcmp(path, default_path) == 0) {
                            // update index for the default history file
                            history_default_file_index = history_length;
                        }
                        else{
                            // update index for the current file
                            history_file_index = history_length;
                        }
                    }
                }
                return 1;
            }

            // Case 2: Read (-r)
            if (strcmp(flag, "-r") == 0) {
                if (target_file && access(path, F_OK) != 0) {
                    printf("history: %s: No such file or directory\n", path);
                    return 1;
                }
                read_history(path);
                return 1;
            }

            // Case 3: Write (-w) -> Overwrite
            if (strcmp(flag, "-w") == 0) {
                write_history(path);
                history_file_index = history_length;
                return 1;
            }
        }

        // List History 
        int base = history_base; 
        int length = history_length;
        int start_offset = 0;

        // If argument is a number
        if (args[1] != NULL && args[1][0] != '-') {
            int n = atoi(args[1]);
            if (n > 0 && n < length) start_offset = length - n;
        }

        for (int i = start_offset; i < length; i++) {
            int display_num = base + i; 
            HIST_ENTRY *entry = history_get(base + i);
            if (entry) printf("%5d  %s\n", display_num, entry->line);
        }
        return 1;
    }

   
    // echo
    if (strcmp(args[0], "echo") == 0)
    {
        for (int i = 1; args[i]; i++)
        {
            printf("%s", args[i]);
            if (args[i + 1])
                printf(" ");
        }
        printf("\n");
        return 1;
    }

    // pwd
    if (strcmp(args[0], "pwd") == 0)
    {
        // print the current working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)))
        {
            printf("%s\n", cwd);
        }
        return 1;
    }

    // cd
    if (strcmp(args[0], "cd") == 0)
    {
        // change directory
        if (args[1] == NULL || strcmp(args[1], "~") == 0)
        {
            char *home = getenv("HOME"); // fetch home directory
            // HOME is an environmental varaiable
            if (!home)
            {
                printf("cd: HOME not set\n");
            }
            else if (chdir(home) != 0)
            { // change to home directory
                // chdir understands absolute and relative paths
                printf("cd: %s: No such file or directory\n", home);
            }
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                printf("cd: %s: No such file or directory\n", args[1]);
            }
        }
        return 1;
    }

    // type
      if (strcmp(args[0], "type") == 0)
      {

        if (args[1] == NULL)
        {
          return 1;
        }
        if (is_builtin(args[1]))
        {
          printf("%s is a shell builtin\n", args[1]);
          return 1;
        }

        // if not a shell builtin
        // go through every directory in the path
        // for each directory check if the filename exit and if it has execute permission print the command,
        // if not skip ahead
        // arg[1] is the file name
        char *path_env = getenv("PATH"); // don't modify it
        char *path = strdup(path_env);
        char *dir = strtok(path, ":");
        int found = 0;

        while (dir != NULL)
        {
          char full_path[1024];
          snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[1]);

          // if this exits and admin priviliges
          if (access(full_path, X_OK) == 0)
          {
            printf("%s is %s\n", args[1], full_path);
            found = 1;
            break;
          }
          // if no admin priviliges skip
          dir = strtok(NULL, ":");
        }
        if (!found)
        {
          // if executable not found
          printf("%s: not found\n", args[1]);
        }

        free(path);
        return 1;
      }

    return 0;
}