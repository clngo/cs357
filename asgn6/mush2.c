#include "mush.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define READ_END 0
#define WRITE_END 1
#define NEWLINE "\n"
#define PROMPT "8-P "
#define PROMPTSIZE  4
#define FILEPERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)


/* Global Variables */
pipeline pline;
int prev_pipe[2];
int next_pipe[2];
int interactive = 0;
int handle_prompt = 0;

void safe_write(int fd, const void* buff, size_t count) {
    if (write(fd, buff, count) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

void safe_dup2(int oldfd, int newfd) {
    if (dup2(oldfd, newfd) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

void handler(int signum) {
    if (interactive) {
        handle_prompt = 1;
        safe_write(STDOUT_FILENO, NEWLINE, 1);
        safe_write(STDOUT_FILENO, PROMPT, PROMPTSIZE);
    }
}

void execute_stage(clstage stage, int isFirst, int isLast, int children) {
    handle_prompt = 0;
    pid_t pid;
    if (pipe(next_pipe) < 0) {
        perror("next pipe");
        exit(EXIT_FAILURE);
    }
    if (strcmp(stage->argv[0], "cd") == 0) {
        if (stage->argv[1] != NULL) {
            if (chdir(stage->argv[1]) != 0) {
                perror(stage->argv[1]);
            }
        }
        else {
            if (chdir(getenv("HOME")) != 0) {
                perror("cd");
            }
        }
        if (interactive) {
            safe_write(STDOUT_FILENO,PROMPT,PROMPTSIZE);
        }
        return;
    }

    if (strcmp(stage->argv[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    }

    pid = fork();

    if (pid == 0) {
        /* Child Process */
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sa.sa_flags = 0;
        if (sigaction(SIGINT, &sa, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
        /* Input redirection */
        if (stage->inname != NULL) {
            /* Take input from a file */
            int infile = open(stage->inname, O_RDONLY);
            if (infile < 0) {
                perror("mush2: read");
                exit(EXIT_FAILURE);
            }
            safe_dup2(infile, STDIN_FILENO);
            close(prev_pipe[READ_END]);
            close(prev_pipe[WRITE_END]);
        } 
        else if (isFirst) {
            /* Take input from stdin */
            close(prev_pipe[WRITE_END]);
            close(prev_pipe[READ_END]);
        } 
        else {
            /* Take input from the previous pipe */
            safe_dup2(prev_pipe[READ_END], STDIN_FILENO);
            close(prev_pipe[WRITE_END]);
        }

        /* Output redirection */
        if (stage->outname != NULL) {
            /* Write to a file */
            int outfile = open(stage->outname, O_WRONLY | O_CREAT | O_TRUNC, FILEPERMISSIONS);
            if (outfile == -1) {
                perror("mush2: open");
                exit(EXIT_FAILURE);
            }
            safe_dup2(outfile, STDOUT_FILENO);
            close(next_pipe[WRITE_END]);
            close(next_pipe[READ_END]);
        } 
        else if (isLast) {
            /* Last child puts output to stdout */
            close(next_pipe[WRITE_END]);
            close(next_pipe[READ_END]);
        } 
        else {
            /* Put output into the next pipe */
            safe_dup2(next_pipe[WRITE_END], STDOUT_FILENO);
            close(next_pipe[READ_END]);
        }


        if (execvp(stage->argv[0], stage->argv) == -1) {
            perror(stage->argv[0]);
            _exit(EXIT_FAILURE);
        }
        
    } 
    else if (pid > 0) { 
        /* Parent process */
        close(prev_pipe[READ_END]);
        close(prev_pipe[WRITE_END]);
        close(next_pipe[WRITE_END]);

        if (!isLast) {
            prev_pipe[READ_END] = next_pipe[READ_END]; 
        }
        int status;
        if (isLast) {
            close(prev_pipe[READ_END]);
            int i = 0;
            while (i < children) { 
                if (wait(&status) == -1 && errno != EINTR) {
                    perror("mush2: wait");
                    exit(EXIT_FAILURE);
                }
                else if (errno != EINTR) {
                    i++;
                }
                else {
                    errno = 0;
                }
            }
            if (interactive && !handle_prompt) {
                safe_write(STDOUT_FILENO,PROMPT,PROMPTSIZE);
            }
        }
        handle_prompt = 0;

    } 
    else {
        perror("mush2: fork");
        exit(EXIT_FAILURE);
    }
}

void execute_pipeline(pipeline pline) {
    int i = 0;
    while (i < pline->length) {
        clstage stage = &(pline->stage[i]);
        execute_stage(stage, i == 0, i == pline->length - 1, i+1);
        i++;
    }
    prev_pipe[READ_END] = -1;
    prev_pipe[WRITE_END] = -1;

    next_pipe[READ_END] = -1;
    next_pipe[WRITE_END] = -1;
}

int main(int argc, char *argv[]) {
    pline = NULL;
    prev_pipe[READ_END] = -1;
    prev_pipe[WRITE_END] = -1;

    next_pipe[READ_END] = -1;
    next_pipe[WRITE_END] = -1;

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    errno = 0;
    if (argc > 1) {
        /* Batch mode: Read commands from a file */ 
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("mush2: fopen");
            exit(EXIT_FAILURE);
        }

        char *line;
        while ((line = readLongString(file)) != NULL) {
            errno = 0;
            pline = crack_pipeline(line);
            if (pline == NULL && errno != 0) {
                perror("crack pipeline");
                exit(EXIT_FAILURE);
            }
            free(line);
            if (pline != NULL) {
                execute_pipeline(pline);
                free_pipeline(pline);
            }
            
        }
        fclose(file);
    } 
    else {
        /* Interactive mode: Read commands from stdin until EOF */
        interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
        if (interactive) {
            safe_write(STDOUT_FILENO, PROMPT, PROMPTSIZE);
            fflush(stdout);
        }
        char *line;
        while ((line = readLongString(stdin)) ||  errno == EINTR) {
            if (errno != EINTR) {
                errno = 0;
                pline = crack_pipeline(line);
                if (pline == NULL && errno != 0) {
                    perror("crack pipeline");
                    exit(EXIT_FAILURE);
                }
                free(line);
                if (pline != NULL) {
                    execute_pipeline(pline);
                    free_pipeline(pline);
                }
                else {
                    safe_write(STDOUT_FILENO, PROMPT, PROMPTSIZE);
                }
            }
            else {
                errno = 0;
            }
            
        }
    }
    return 0;
}
