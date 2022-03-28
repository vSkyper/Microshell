#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define BUFFER_SIZE 1024
#define CYAN "\x1B[36m"
#define BLUE "\x1B[34m"
#define YELLOW "\x1B[33m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define COLOR_RESET "\x1B[0m"

char *split(char *str, char **next) {
    char *current = str;
    char *start = str;
    int number = 0;

    while (*current && (*current == ' ')) {
        current++;
        number++;
    }

    start = current;

    while (1) {
        if ((*(current - 1) == '\\') && (*current == '"')) {
            memmove(&start[number - 1], &start[number],
                    strlen(start) - (number - 1));
        } else if ((*(current - 1) != '\\') && (*current == '"')) {
            memmove(&start[number], &start[number + 1], strlen(start) - number);

            while (1) {
                if ((*(current - 1) == '\\') && (*current == '"')) {
                    memmove(&start[number - 1], &start[number],
                            strlen(start) - (number - 1));
                } else if ((*(current - 1) != '\\') && (*current == '"')) {
                    memmove(&start[number], &start[number + 1],
                            strlen(start) - number);
                    break;
                } else {
                    current++;
                    number++;
                }
            }
        } else if ((*current == ' ') || (!*current)) {
            break;
        } else {
            current++;
            number++;
        }
    }

    if (*current) {
        *current = '\0';
        current++;

        while (*current && (*current == ' ')) {
            current++;
        }
    }

    *next = current;

    return start;
}

void help() {
    printf(YELLOW "\nMicroshell author: " COLOR_RESET "Mateusz Piwowarski\n");
    printf(YELLOW
           "\nCommands:" COLOR_RESET
           "\n>> exit\n>> help\n>> cd\n>> mv\n>> mkdir: flags: -v, -p, "
           "-vp/-pv\n>>Commands that refer by name to programs located in "
           "directories described by the value of the PATH environment "
           "variable, and allow these scripts and programs to be called with "
           "arguments\n>> Support for quotation marks: \"...\"\n");
}

void cd(char *StringArray[BUFFER_SIZE], char prev_directory_main[BUFFER_SIZE],
        char prev_directory_secondary[BUFFER_SIZE]) {
    getcwd(prev_directory_main, BUFFER_SIZE);

    if (StringArray[1] == NULL || strcmp(StringArray[1], "~") == 0) {
        if (chdir(getenv("HOME")) < 0) {
            perror(RED "cd" COLOR_RESET);
            strcpy(prev_directory_main, prev_directory_secondary);
        }
    } else if (strcmp(StringArray[1], "-") == 0) {
        if (chdir(prev_directory_secondary) < 0) {
            perror(RED "cd" COLOR_RESET);
            strcpy(prev_directory_main, prev_directory_secondary);
        } else {
            printf("%s\n", prev_directory_secondary);
        }
    } else {
        if (chdir(StringArray[1]) < 0) {
            perror(RED "cd" COLOR_RESET);
            strcpy(prev_directory_main, prev_directory_secondary);
        }
    }
}

void mv(char *StringArray[BUFFER_SIZE], int numberOfArguments) {
    char tok_mv[BUFFER_SIZE];
    char *tok_mv2;
    char *ptr_mv[BUFFER_SIZE];
    char do_mv[BUFFER_SIZE];
    DIR *dir;

    if (StringArray[1] == NULL || StringArray[2] == NULL) {
        printf(RED "mv" COLOR_RESET ": Not enough arguments");
    } else {
        if ((dir = opendir(StringArray[numberOfArguments - 1])) == NULL) {
            if (rename(StringArray[1], StringArray[2]) < 0) {
                perror(RED "mv" COLOR_RESET);
            }
        } else {
            int j;
            for (j = 1; j < numberOfArguments - 1; j++) {
                int k = 0;
                strcpy(do_mv, StringArray[j]);
                strcpy(tok_mv, StringArray[numberOfArguments - 1]);
                int len = strlen(tok_mv);
                if (tok_mv[len - 1] != '/') {
                    strcat(tok_mv, "/");
                }

                tok_mv2 = strtok(do_mv, "/");
                while (tok_mv2) {
                    ptr_mv[k] = tok_mv2;
                    tok_mv2 = strtok(NULL, "/");
                    k++;
                }

                strcat(tok_mv, ptr_mv[k - 1]);
                if (rename(StringArray[j], tok_mv) < 0) {
                    perror(RED "mv" COLOR_RESET);
                }
            }

            closedir(dir);
        }
    }
}

void mkdir_fun(char *StringArray[BUFFER_SIZE], int numberOfArguments) {
    char *tok_mkdir;
    char *ptr_mkdir[BUFFER_SIZE];
    char do_mkdir[BUFFER_SIZE];
    DIR *dir;

    umask(0);
    if (strcmp(StringArray[1], "-v") == 0) {
        int j;
        for (j = 2; j < numberOfArguments; j++) {
            if (mkdir(StringArray[j], 0777) < 0) {
                perror(RED "mkdir" COLOR_RESET);
            } else {
                printf(GREEN "Directory created" COLOR_RESET ": \'%s\'\n",
                       StringArray[j]);
            }
        }
    } else if (strcmp(StringArray[1], "-p") == 0) {
        int j;
        for (j = 2; j < numberOfArguments; j++) {
            int k = 0;

            tok_mkdir = strtok(StringArray[j], "/");
            while (tok_mkdir) {
                ptr_mkdir[k] = tok_mkdir;
                tok_mkdir = strtok(NULL, "/");
                k++;
            }

            strcpy(do_mkdir, StringArray[j]);

            if ((dir = opendir(do_mkdir)) == NULL) {
                if (mkdir(do_mkdir, 0777) < 0) {
                    perror(RED "mkdir" COLOR_RESET);
                }
            } else {
                closedir(dir);
            }

            int x = 1;
            for (x = 1; x < k; x++) {
                strcat(do_mkdir, "/");
                strcat(do_mkdir, ptr_mkdir[x]);
                if ((dir = opendir(do_mkdir)) == NULL) {
                    if (mkdir(do_mkdir, 0777) < 0) {
                        perror(RED "mkdir" COLOR_RESET);
                    }
                } else {
                    closedir(dir);
                }
            }
        }
    } else if ((strcmp(StringArray[1], "-pv") == 0) ||
               (strcmp(StringArray[1], "-vp") == 0)) {
        int j;
        for (j = 2; j < numberOfArguments; j++) {
            int k = 0;

            tok_mkdir = strtok(StringArray[j], "/");
            while (tok_mkdir) {
                ptr_mkdir[k] = tok_mkdir;
                tok_mkdir = strtok(NULL, "/");
                k++;
            }

            strcpy(do_mkdir, StringArray[j]);

            if ((dir = opendir(do_mkdir)) == NULL) {
                if (mkdir(do_mkdir, 0777) < 0) {
                    perror(RED "mkdir" COLOR_RESET);
                } else {
                    printf(GREEN "Directory created" COLOR_RESET ": \'%s\'\n",
                           do_mkdir);
                }
            } else {
                closedir(dir);
            }

            int x = 1;
            for (x = 1; x < k; x++) {
                strcat(do_mkdir, "/");
                strcat(do_mkdir, ptr_mkdir[x]);
                if ((dir = opendir(do_mkdir)) == NULL) {
                    if (mkdir(do_mkdir, 0777) < 0) {
                        perror(RED "mkdir" COLOR_RESET);
                    } else {
                        printf(GREEN "Directory created" COLOR_RESET
                                     ": \'%s\'\n",
                               do_mkdir);
                    }
                } else {
                    closedir(dir);
                }
            }
        }
    } else {
        int j;
        for (j = 1; j < numberOfArguments; j++) {
            if (mkdir(StringArray[j], 0777) < 0) {
                perror(RED "mkdir" COLOR_RESET);
            }
        }
    }
}

int main() {
    char cwd[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    char *user;
    char *StringArray[BUFFER_SIZE];
    char *token;
    char prev_directory_main[BUFFER_SIZE];
    char prev_directory_secondary[BUFFER_SIZE];

    getcwd(prev_directory_main, BUFFER_SIZE);

    while (1) {
        strcpy(prev_directory_secondary, prev_directory_main);

        getcwd(cwd, BUFFER_SIZE);

        user = getenv("USER");

        printf("\n[" CYAN "%s" COLOR_RESET ":" BLUE "%s" COLOR_RESET "]\n$ ",
               user, cwd);

        fgets(command, BUFFER_SIZE, stdin);

        command[strcspn(command, "\r\n")] = '\0';

        token = command;

        int numberOfArguments = 0;

        while (*token) {
            StringArray[numberOfArguments] = split(token, &token);
            numberOfArguments++;
        }

        StringArray[numberOfArguments] = NULL;

        if (StringArray[0] == NULL || strcmp(StringArray[0], "") == 0) {
            continue;
        }

        if (strcmp(command, "exit") == 0) {
            exit(EXIT_SUCCESS);
        } else if (strcmp(command, "help") == 0) {
            help();
        } else if (strcmp(StringArray[0], "cd") == 0) {
            cd(StringArray, prev_directory_main, prev_directory_secondary);
        } else if (strcmp(StringArray[0], "mv") == 0) {
            mv(StringArray, numberOfArguments);
        } else if (strcmp(StringArray[0], "mkdir") == 0) {
            mkdir_fun(StringArray, numberOfArguments);
        } else {
            pid_t id = fork();

            if (id < 0) {
                perror(RED "Microshell" COLOR_RESET);
            } else if (id == 0) {
                if (execvp(StringArray[0], StringArray) < 0) {
                    perror(RED "Microshell" COLOR_RESET);
                    exit(EXIT_FAILURE);
                }
            } else {
                wait(NULL);
            }
        }
    }
}