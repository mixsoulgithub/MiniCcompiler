#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <program file>\n", argv[0]);
        return 1;
    }
    char *p = argv[1];
    printf("  .globl main\n");
    printf("main:\n");
    printf("  mov $%ld, %%rax\n", strtol(p, &p, 10));
    while (*p) {//not p! p is a pointer, so while never ends.
        if (*p == '+') {
            p++;
            printf("  add $%ld, %%rax\n", strtol(p, &p, 10));
            continue;
        }
        if (*p == '-') {
            p++;
            printf("  sub $%ld, %%rax\n", strtol(p, &p, 10));
            continue;
        }
        fprintf(stderr, "unexpected character: '%c'\n", *p);
        return 1;
    }
    printf("  ret\n");
    return 0;
}