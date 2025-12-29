#include <readline/readline.h>
#include <readline/history.h>
#include "parser.h"
#include "executor.h"
#include "completion.h"
#include <stdlib.h>
#include "builtins.h"

int main() {

    init_completion(); // TAB completion
    load_history();
    atexit(save_history_cleanup);

    while (1) {
        char *line = readline("$ ");
        if (!line){
          printf("\n");
          break;
        }

        if (*line) add_history(line);

        char *args[32];
        parser(line, args);
        if(args[0] == NULL){
          free(line);
          continue;
        }
        execute(args);

        free(line);
    }

    return 0;
}

