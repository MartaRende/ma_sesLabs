#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int global_init = 42;
int global_uninit;

void foo(void) { }

int main(void)
{
    int stack_var = 0;
    void *heap = malloc(64);

    printf("Address of main (text)        : %p\n", (void*)main);
    printf("Address of foo  (text)        : %p\n", (void*)foo);
    printf("Address of stack_var (stack)  : %p\n", (void*)&stack_var);
    printf("Address of heap (malloc)      : %p\n", heap);
    printf("Address of printf (libc)      : %p\n", (void*)printf);
    free(heap);
    return 0;
}
