#include "wish.h"
#include <ctype.h>  // isspace
#include <regex.h>  // regcomp, regexec, regfree
#include <stdio.h>  // fopen, fclose, fileno, getline, feof
#include <stdlib.h> // exit
#include <sys/types.h>
#include <sys/wait.h> // waitpid


int execute_command(char *line) {
    printf("EXECUTE!! %s\n", line);

    //   if (strcmp(exit, line) == 0) {
    //     break;
    //   } else {
    //     printf("%i", strcmp(exit, line));

    return 0;
}


int main(int argc, char *argv[]) {
  
    // const char exit[] = "exit";
    int id = 1;
    int error_code = 0;

    char *line = NULL;
    size_t len = 0;

    printf("wish> ");
    getline(&line, &len, stdin);

      
    id = fork();

    if (id > 0) {
        waitpid(id, NULL, 0);
    } else if (id == 0) {
        execute_command(line);
    } else {
        error_code = -1;
    }

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