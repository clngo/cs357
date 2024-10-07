#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#define BUFFER_SIZE 10
#define NULL_CHAR '\0'
#define NEWLINE_CHAR '\n'

char *read_long_line(FILE *file); 

int main(int argc, char *argv[]) {
    char *res;
    char *tmp = NULL;
    res = read_long_line(stdin);
    
    if (res == NULL) {
        return 0;
    }

    while (res) {
        // Remove line terminators from res for comparison
        size_t length = strlen(res);
        if (length > 0 && (res[length - 1] == NEWLINE_CHAR)) {
            res[length - 1] = NULL_CHAR;
        }

        if (tmp == NULL || strcmp(res, tmp)) {
            printf("%s%c", res, NEWLINE_CHAR);
            free(tmp);
            tmp = strdup(res); // Create a copy of res
        }
        free(res);
        res = read_long_line(stdin);
    }

    free(tmp);
    return 0;
}

void *safe_malloc(size_t size) {
    void *new;
    if (!(new = malloc(size))) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return new;
}

void *safe_realloc(char *line, size_t size) {
    void *new;
    if (!(new = realloc(line, size))) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return new;
}

char *read_long_line(FILE *file) {
    char *line;
    char *new_line;
    size_t size = BUFFER_SIZE; // Define MALL appropriately
    size_t length = 0;

    line = safe_malloc(size);

    while (fgets(line + length, size - length, file)) {
        length += strlen(line + length);
        // Normalize line terminators
        if (line[length - 1] == NULL_CHAR || line[length - 1] == NEWLINE_CHAR) {
            // line[length - 1] = NEWLINE_CHAR;
            break;
        }

        // if (line[length - 1] == NEWLINE_CHAR) {
        //     line[length] = NULL_CHAR; // Null-terminate the line
        //     break;
        // }

        else {
        // if (size - length <= 1) {
            size += BUFFER_SIZE; // Adjust MALL as needed
            new_line = safe_realloc(line, size);
            if (new_line == NULL) {
                // Handle memory allocation failure
                free(line);

                return NULL;
            }
            line = new_line;
        }
    }

    if (length == 0) {
        free(line);
        return NULL;
    }

    return line;
}

