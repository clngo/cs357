#include <stdio.h>

int main() {
    char s[] = "Hello, world!\n";
    char *p;
    for (p = s; *p != '\0'; p++) {
        putchar(*p);
    }
    printf("%zu", sizeof(size_t));
    return 0;
}
