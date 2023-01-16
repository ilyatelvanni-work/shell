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


struct ConsoleCommand parse_command(const char * const line) {

    printf("parse_command got: '%s'\n", line);

    char *line_editable = malloc(sizeof(char) * (strlen(line)));
    strcpy(line_editable, line);

    char *delim = " ";
    char default_path[] = "/bin/";
    char *path = default_path;

    char *command_token = strtok(line_editable, delim);
    printf("got command_token '%s' for execution\n", command_token);

    char *command = malloc(sizeof(char) * (strlen(path) + strlen(command_token)));
    strcpy(command, path);
    strcat(command, command_token);
    printf("command '%s' in '%p' is generated\n", command, command);


    int args_number = 0;
    char *args[1000] = {};
    
    do {
        command_token = strtok(NULL, delim);

        if (command_token != NULL) {
            char *arg = malloc(sizeof(char) * strlen(command_token));
            args[args_number++] = arg;

            printf("got %i common token: %s\n", args_number, command_token);
        }
    } while(command_token != NULL);

    free(line_editable);

    struct ConsoleCommand result;
    result.command = command;
    result.args = malloc(sizeof(char*) * (args_number + 1));
    for (int i = 0;i < args_number;i++) {
        result.args[i] = args[i];
    }
    result.args[args_number] = NULL;
    result.args_number = args_number;

    return result;
}


int execute_command(const char * const line) {

    struct ConsoleCommand command = parse_command(line);

    printf("command for execution: '%s' in '%p' \n", command.command, command.command);
    printf("DEBUG: %s", command.args[0]);

    int result = execv(command.command, command.args);
    printf("command execution result code: %i\n", result);

    //   if (strcmp(exit, line) == 0) {
    //     break;
    //   } else {
    //     printf("%i", strcmp(exit, line));

    return 0;
}


int main(int argc, char *argv[]) {

    // const char command_test[] = "ls -ll";

    // execute_command(command_test);

  
    // 
    int id = 1;
    int error_code = 0;

    char *line = NULL;
    size_t len = 0;

    printf("wish> ");
    getline(&line, &len, stdin);

      
    // id = fork();

    execute_command(line);

    // if (id > 0) {
    //     waitpid(id, NULL, 0);
    // } else if (id == 0) {
    //     execute_command(line);
    // } else {
    //     error_code = -1;
    // }

    if (error_code) {
        printError();
        return id;
    }


      //       //build-in_programms
      //       //exit -> exit(0)
      //       //cd <arg?> -> chdir(<args?>)
      //       //path <args> -> set path of the shell (instead
      //       //   of /bin /usr/bin)

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