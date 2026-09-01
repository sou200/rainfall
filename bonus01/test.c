#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    char a[16] = {0};
    char b[16] = {0};

    read(STDIN_FILENO, a, 16);
    read(STDIN_FILENO, b, 16);
    // puts("---------------");
    printf("%s-%s", a, b);
    return 0;
}