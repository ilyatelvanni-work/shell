#include "wish.h"
#include <ctype.h>  // isspace
#include <regex.h>  // regcomp, regexec, regfree
#include <stdio.h>  // fopen, fclose, fileno, getline, feof
#include <stdlib.h> // exit
#include <sys/types.h>
#include <sys/wait.h> // waitpid


struct ConsoleCommand {
    char* command;
    char** args;
    int args_number;
};
const char EXIT_COMMAND[] = "exit";
const char EXIT_COMMAND_ENTER[] = "exit\n";

const int PRINT_LOGS = 1;
const int PRINT_DEBUG = 1;

char* bin_path_list[] = {"/bin/", NULL};


char* make_build_in_command(const char * const command_token) {

    int i = 0;
    FILE *file;

    while(bin_path_list[i] != NULL) {
        char *command = malloc(sizeof(char) * (strlen(bin_path_list[i]) + strlen(command_token)));

        strcpy(command, bin_path_list[i]);
        strcat(command, command_token);

        if ((file = fopen(command,"r")) != NULL) {
            fclose(file);
            return command;
        } else {
            i++;
            if (PRINT_LOGS) printf("%s file doesn't exists\n", command);
        }
    }

    return NULL;
}


struct ConsoleCommand parse_command(const char * const line) {

    struct ConsoleCommand result;

    char *line_editable = malloc(sizeof(char) * strlen(line));
    strcpy(line_editable, line);

    if (strlen(line) != 0 && line_editable[strlen(line) - 1] == '\n'){
        line_editable[strlen(line) - 1] = '\0';
    }

    if (PRINT_LOGS) printf("parse_command got: '%s'\n", line_editable);

    char *delim = " ";
    char *command_token = strtok(line_editable, delim);
    if (PRINT_LOGS) printf("got command_token '%s' for execution\n", command_token);

    char *command = make_build_in_command(command_token);
    if (PRINT_LOGS) printf("command '%s' in '%p' is generated\n", command, command);
    if (command == NULL) {
        result.command = NULL;
        return result;
    }

    int args_number = 0;
    char *args[1000] = {};
    
    do {
        command_token = strtok(NULL, delim);

        if (command_token != NULL) {
            char *arg = malloc(sizeof(char) * strlen(command_token));
            strcpy(arg, command_token);
            args[args_number++] = arg;

            if (PRINT_LOGS) printf("got %i command token in '%p': %s\n", args_number, arg, arg);
        }
    } while(command_token != NULL);

    free(line_editable);

    result.command = command;

    result.args = malloc(sizeof(char*) * (args_number + 2));
    result.args[0] = result.command;
    for (int i = 0;i < args_number;i++) {
        result.args[i + 1] = args[i];
    }
    result.args[args_number + 1] = NULL;
    result.args_number = args_number;

    return result;
}


int execv_in_thread(const char * const command, char * const * const args) {
    int id = fork();

    if (id > 0) {
        waitpid(id, NULL, 0);
        return 0;
    } else if (id == 0) {
        return execv(command, args);
    } else {
        if (PRINT_LOGS) printf("ERROROROROROROROR\n");
        return -1;
    }
}


int execute_command(const char * const line) {
    if (strcmp(EXIT_COMMAND, line) == 0 || strcmp(EXIT_COMMAND_ENTER, line) == 0) {
        if (PRINT_LOGS) printf("got exit command\n");
        
        return -1;
    } else {
        struct ConsoleCommand command = parse_command(line);

        if (PRINT_LOGS) printf("command for execution: '%s' in '%p' \n", command.command, command.command);
        if (command.command == NULL) {
            return 1;
        }

        for (int i = 0;i < command.args_number + 1;i++) {
            if (PRINT_LOGS && PRINT_DEBUG) printf("DEBUG: arg '%i' in '%p': '%s'\n", i, command.args[i], command.args[i]);
        }
        if (PRINT_LOGS && PRINT_DEBUG) printf("\n");

        int result = execv_in_thread(command.command, command.args);
        if (PRINT_LOGS) printf("command execution result code: %i\n", result);

        return result;
    }
}


int main(int argc, char *argv[]) {

    int execution_code = 0;

    char *line = NULL;
    size_t len = 0;

    while(execution_code == 0) {
        printf("wish> ");
        getline(&line, &len, stdin);
        if (PRINT_LOGS) printf("get command '%s'\n", line);

        execution_code = execute_command(line);
        if (PRINT_LOGS) printf("command executed, code %i\n", execution_code);
    }

    if (execution_code > 0) {
        printError();
    }


    //       // build-in_programms
    //       // exit -> exit(0)
    //       // cd <arg?> -> chdir(<args?>)
    //       // path <args> -> set path of the shell (instead
    //       //     of /bin /usr/bin)

    //       // commands-in-path
    //       // check bin
    //       // access("/bin/ls", X_OK)
    //       // /usr/bin/l

    //       // redirection
    //       // > print nothing on the screen print in file
    //       // (overwrite)
    //       // no redirection for build ups
    //       // no redirection chains

    //       // parallel commands
    //       // & execute commands without waiting one-by-one
    //       // but execute waiting whem simultaneously exec

    //       // char error_message[30] = "An error has occurred\n";
    //       // write(STDERR_FILENO, error_message, strlen(error_message)); 
    //       // CATCH ALL SYNTAX ERRORS EITHER

    return 0;
}