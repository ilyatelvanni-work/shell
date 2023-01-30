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
    char** redirections;
};
const char EXIT_COMMAND[] = "exit";
const char CD_COMMAND[] = "cd";
const char PATH_COMMAND[] = "path";
const char EXIT_COMMAND_ENTER[] = "exit\n"; // TODO: REMOVE

const int PRINT_LOGS = 0;
const int PRINT_DEBUG = 0;

char DEFAULT_PATH_1[] = "/bin";
char DEFAULT_PATH_2[] = "/usr/bin";

char** bin_path_vector;

const char REDIRECTION_ARG[] = ">";


void init_default_path() {
    bin_path_vector = malloc(sizeof(char*) * 3);
    bin_path_vector[0] = DEFAULT_PATH_1;
    bin_path_vector[1] = DEFAULT_PATH_2;
    bin_path_vector[2] = NULL;
}


char* make_build_in_command(const char * const command_token) {

    int i = 0;
    FILE *file;

    if (strcmp(CD_COMMAND, command_token) == 0) {
        char *command = malloc(sizeof(char) * strlen(CD_COMMAND));
        strcpy(command, CD_COMMAND);

        return command;
    } else if (strcmp(PATH_COMMAND, command_token) == 0) {
        char *command = malloc(sizeof(char) * strlen(PATH_COMMAND));
        strcpy(command, PATH_COMMAND);

        return command;
    }

    while(bin_path_vector[i] != NULL) {
        char *command = malloc(sizeof(char) * (strlen(bin_path_vector[i]) + 1 + strlen(command_token)));

        strcpy(command, bin_path_vector[i]);
        strcat(command, "/");
        strcat(command, command_token);

        if ((file = fopen(command,"r")) != NULL) {
            fclose(file);
            return command;
        } else {
            i++;
            if (PRINT_LOGS) printf("%s file doesn't exists\n", command);
            free(command);
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
        free(line_editable);
        return result;
    }

    char *args[1000] = {};
    char *redirections[1000] = {};
    int args_number = 0;
    int redirection_number = 0;
    int redirection_arg = 0;

    char* original_command_name = malloc(sizeof(char) * strlen(command_token));
    strcpy(original_command_name, command_token);
    
    do {
        command_token = strtok(NULL, delim);

        if (command_token != NULL) {
            if (strcmp(REDIRECTION_ARG, command_token)) {
                char *arg = malloc(sizeof(char) * strlen(command_token));
                strcpy(arg, command_token);

                if (redirection_arg) {
                    redirections[redirection_number++] = arg;
                    if (PRINT_LOGS) printf("got %i redirection token in '%p': %s\n", redirection_number, arg, arg);
                } else {
                    args[args_number++] = arg;
                    if (PRINT_LOGS) printf("got %i command token in '%p': %s\n", args_number, arg, arg);
                }
            } else {
                redirection_arg = 1;
            }
        }
    } while(command_token != NULL);

    free(line_editable);

    // if (redirection_arg == 1) {
    //     result.command = NULL;
    //     return result;
    // }

    result.command = command;

    result.args = malloc(sizeof(char*) * (args_number + 2));
    result.args[0] = original_command_name;
    for (int i = 0;i < args_number;i++) {
        result.args[i + 1] = args[i];
    }
    result.args[args_number + 1] = NULL;

    result.redirections = malloc(sizeof(char*) * (redirection_number + 1));
    for (int i = 0;i < redirection_number;i++) {
        result.redirections[i] = redirections[i];
    }
    result.redirections[redirection_number] = NULL;

    return result;
}


int execv_in_thread(const char * const command, char * const * const args, char * const * const redirections) {
    int pipefd[2];
    pipe(pipefd);
    int id = fork();

    if (id > 0) {
        id = waitpid(id, NULL, 0);
        if (id == -1) {
            return -1;
        } else {
            char buffer[1024];
            close(pipefd[1]);

            int redirections_number = 0;
            for (;redirections[redirections_number] != NULL;redirections_number++);

            FILE **redirection_files = malloc(sizeof(FILE*) * redirections_number);
            for (int i = 0;i < redirections_number;i++) {
                redirection_files[i] = fopen(redirections[i], "w");
            }

            while (read(pipefd[0], buffer, sizeof(buffer)) != 0) {
                if (redirections_number == 0) {
                    printf("%s", buffer);
                } else {
                    for (int i = 0;i < redirections_number;i++) {
                        fprintf(redirection_files[i], "%s", buffer);
                    }
                }
            }

            return 0;
        }
    } else if (id == 0) {

        close(pipefd[0]);    // close reading end in the child

        dup2(pipefd[1], 1);  // send stdout to the pipe
        dup2(pipefd[1], 2);  // send stderr to the pipe

        close(pipefd[1]);    // this descriptor is no longer needed

        int result = execv(command, args);
        return result;
    } else {
        if (PRINT_LOGS) printf("ERROROROROROROROR\n");
        return -1;
    }
}


int execute_cd_command(const struct ConsoleCommand command) {
    return command.args[1] == NULL || command.args[2] != NULL ? -1 : chdir(command.args[1]);
}


int execute_path_command(const struct ConsoleCommand command) {
    if (PRINT_LOGS) printf("executing path command, bin_path_vector now is in '%p'\n", bin_path_vector);

    if (command.args[1] == NULL) {
        if (PRINT_LOGS) printf("got path with no arguments\n");
        bin_path_vector = malloc(sizeof(char*));
        bin_path_vector[0] = NULL;
    } else {
        int args_len = 0;
        int bin_path_len = 0;
        for (;command.args[args_len + 1] != NULL;args_len++);
        for (;bin_path_vector[bin_path_len] != NULL;bin_path_len++);

        int bin_path_len_new = bin_path_len + args_len;

        if (PRINT_LOGS) printf("bin_path_vector's length now is %i\n", bin_path_len_new);

        char** bin_path_vector_new = malloc(sizeof(char*) * (bin_path_len_new + 1));

        for (int i = 0;bin_path_vector[i] != NULL;i++) bin_path_vector_new[i] = bin_path_vector[i];
        for (int i = 1;command.args[i] != NULL;i++) {
            char* new_path = malloc(sizeof(char) * (strlen(command.args[i]) + 1));
            strcpy(new_path, command.args[i]);
            bin_path_vector_new[bin_path_len + i - 1] = new_path;
        }
        bin_path_vector_new[bin_path_len_new] = NULL;

        bin_path_vector = bin_path_vector_new;
    }

    int i = 0;
    do {
        if (PRINT_LOGS && PRINT_DEBUG) printf("DEBUG: bin_path_vector[%i] now is '%s'\n", i, bin_path_vector[i]);
    } while(bin_path_vector[i++] != NULL);
    if (PRINT_LOGS && PRINT_DEBUG) printf("\nDEBUG: bin_path_vector is in '%p'\n", bin_path_vector);

    return 0;
}


int execute_command(const char * const line) {
    if (strcmp(EXIT_COMMAND, line) == 0 || strcmp(EXIT_COMMAND_ENTER, line) == 0) {
        if (PRINT_LOGS) printf("got exit command\n");
        
        return 1;
    } else {
        struct ConsoleCommand command = parse_command(line);

        if (PRINT_LOGS) printf("command for execution: '%s' in '%p' \n", command.command, command.command);
        if (command.command == NULL) {
            return -1;
        }

        for (int i = 0;1;i++) {
            if (PRINT_LOGS && PRINT_DEBUG) printf("DEBUG: arg '%i' in '%p': '%s'\n", i, command.args[i], command.args[i]);
            if (command.args[i] == NULL) break;
        }
        for (int i = 0;1;i++) {
            if (PRINT_LOGS && PRINT_DEBUG) printf("DEBUG: redirection '%i' in '%p': '%s'\n", i, command.redirections[i], command.redirections[i]);
            if (command.redirections[i] == NULL) break;
        }
        if (PRINT_LOGS && PRINT_DEBUG) printf("\n");

        int result = 0;
        if (strcmp(CD_COMMAND, command.command) == 0) {
            result = execute_cd_command(command);
        } else if (strcmp(PATH_COMMAND, command.command) == 0) {
            result = execute_path_command(command);
        } else {
            result = execv_in_thread(command.command, command.args, command.redirections);
        }

        if (PRINT_LOGS) printf("command execution result code: %i\n", result);

        return result;
    }
}


int execute_interactive_mode() {
    int execution_code = 0;

    char *line = NULL;
    size_t len = 0;

    while(execution_code <= 0) {
        printf("wish> ");
        getline(&line, &len, stdin);
        if (PRINT_LOGS) printf("get command '%s'\n", line);

        execution_code = execute_command(line);
        if (PRINT_LOGS) printf("command executed, code %i\n", execution_code);

        if (execution_code < 0) {
            printError();
        }
    }

    return 0;
}


int execute_batch_mode(const char * const source_path) {
    int execution_code = 0;

    FILE *source = fopen(source_path, "r");

    if (source == NULL) {
        printError();
    } else {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((execution_code <= 0) && ((read = getline(&line, &len, source)) != -1)) {
            if (PRINT_LOGS) printf("get command '%s'\n", line);
            
            execution_code = execute_command(line);
            
            if (PRINT_LOGS) printf("command executed, code %i\n", execution_code);

            if (execution_code < 0) {
                printError();
            }
        }

        fclose(source);
    }

    return 0;
}


int main(int argc, char *argv[]) {

    init_default_path();

    if (argv[1] == NULL) {
        return execute_interactive_mode();
    } else {
        return execute_batch_mode(argv[1]);
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