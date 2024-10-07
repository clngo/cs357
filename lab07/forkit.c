#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    printf("Hello, world!\n");

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        /* Child */
        printf("This is the child, pid %d.\n", getpid());
        exit(EXIT_SUCCESS);
    } else {
        /* Parent */
        int status;
        printf("This is the parent, pid %d\n", getpid());
        waitpid(child_pid, &status, 0);

        printf("This is the parent, pid %d, signing off.\n", getpid());
        exit(EXIT_SUCCESS);
    }
    return 0;
}
