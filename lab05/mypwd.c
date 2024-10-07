#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CURDIR "."
#define PARDIR ".."
#define NULL_CHAR '\0'
#define SLASH '/'
#define STRSLASH "/"

int compareStat(struct stat stat1, struct stat stat2) {
    return ((stat1.st_ino == stat2.st_ino) && (stat1.st_dev == stat2.st_dev));
}

int main(int argc, char *argv[]) {
    char cwd[PATH_MAX] = {NULL_CHAR};
    DIR* curr_dir = opendir(CURDIR);
    if (!curr_dir) {
        perror("mypwd");
        exit(EXIT_FAILURE);
    }
    
    struct stat curStat;
    if (lstat(CURDIR, &curStat) < 0) {
        perror("mypwd");
        exit(EXIT_FAILURE);
    }


    struct stat parStat;
    if (lstat(PARDIR, &parStat) <0) {
        perror("mypwd");
        exit(EXIT_FAILURE);
    }

    struct dirent *parent_dirent;
    DIR* parent_dir = opendir(PARDIR);
    
    /* Keep going up the path, remember the child */
    while (!compareStat(curStat, parStat)) {
        chdir(PARDIR);
        parent_dir = opendir(CURDIR);
        if (parent_dir < 0) {
            perror("mypwd");
            exit(EXIT_FAILURE);
        }

        /* Traverse through each child in the parent directory */
        while ((parent_dirent = readdir(parent_dir))) {
            if (lstat(parent_dirent->d_name, &parStat) < 0) {
                perror("mypwd");
                exit(EXIT_FAILURE);
            }
            if (compareStat(curStat, parStat)) {
                break;
            }
        }
        
        if (!(parent_dirent)) {
            printf("cannot get current directory");
            return 0;
        }
        int cwdLength;
        if ((cwdLength = strlen(cwd))) {
            memmove(cwd+strlen(parent_dirent->d_name)+1,cwd,cwdLength+1);
            cwd[0] = SLASH;
            memmove(cwd+1, parent_dirent->d_name,strlen(parent_dirent->d_name));
        }

        if (!strlen(cwd)) {
            strcat(cwd, STRSLASH);
            strcat(cwd, parent_dirent->d_name);
        }

        /* Wow very long path, it's beyond the max */
        if (strlen(cwd) > PATH_MAX) {
            printf("path too long");
            return 0;
        }

        
        if (lstat(CURDIR, &curStat) < 0) {
            perror("mypwd");
            exit(EXIT_FAILURE);
        }

        if (lstat(PARDIR, &parStat) < 0) {
            perror("mypwd");
            exit(EXIT_FAILURE);
        }

    }
    printf("%s\n", cwd);
    return 0;
}
