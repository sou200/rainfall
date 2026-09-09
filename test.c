#include <stdio.h>

static char a[10];

int main()
{
    write(1, a, 10);
}