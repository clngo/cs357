#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s command\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        /* Child */
        execlp(argv[1], argv[1], NULL);

        perror(argv[1]);
        exit(EXIT_FAILURE);
    } else {
        /* Parent */
        int status;
        waitpid(child_pid, &status, 0);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                printf("Process %d %s.\n", child_pid, "succeeded");   
            }
            else {
                printf("Process %d %s.\n", child_pid, "exited with an error value");
            }
            
        } 
        else {
            fprintf(stderr, "Child process did not terminate normally.\n");
            exit(EXIT_FAILURE);
        }

        exit(WEXITSTATUS(status));
    }
}
