#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "completion.h"

static char **matches = NULL;
static int match_count = 0;

static void clear_matches(void) {
    for (int i = 0; i < match_count; i++)
        free(matches[i]);
    free(matches);
    matches = NULL;
    match_count = 0;
}

static void collect_matches(const char *text) {
    clear_matches();

    static const char *builtins[] = {
        "exit", "echo", "cd", "pwd", "type", NULL
    };

    // builtins 
    for (int i = 0; builtins[i]; i++) {
        if (strncmp(builtins[i], text, strlen(text)) == 0) {
            matches = realloc(matches, sizeof(char*) * (match_count + 1));
            matches[match_count++] = strdup(builtins[i]);
        }
    }

    // PATH executables 
    char *path = strdup(getenv("PATH"));
    for (char *dir = strtok(path, ":"); dir; dir = strtok(NULL, ":")) {
        DIR *d = opendir(dir);
        if (!d) continue;

        struct dirent *ent;
        while ((ent = readdir(d))) {
            if (strncmp(ent->d_name, text, strlen(text)) != 0)
                continue;

            char full[1024];
            snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);

            if (access(full, X_OK) == 0) {
                int dup = 0;
                for (int i = 0; i < match_count; i++) {
                    if (!strcmp(matches[i], ent->d_name)) {
                        dup = 1;
                        break;
                    }
                }
                if (!dup) {
                    matches = realloc(matches, sizeof(char*) * (match_count + 1));
                    matches[match_count++] = strdup(ent->d_name);
                }
            }
        }
        closedir(d);
    }
    free(path);

    qsort(matches, match_count, sizeof(char*),
          (int (*)(const void*, const void*))strcmp);
}

static char *command_generator(const char *text, int state) {
    static int index;

    if (state == 0) {
        index = 0;
        collect_matches(text);
    }

    if (index < match_count)
        return strdup(matches[index++]);

    return NULL;
}

static char **command_completion(const char *text, int start, int end) {
    (void)end;

    // only complete first word 
    if (start != 0)
        return NULL;

    return rl_completion_matches(text, command_generator);
}

void init_completion(void) {
    rl_attempted_completion_function = command_completion;

    // bash-like behavior
    rl_completion_append_character = '\0';
}
