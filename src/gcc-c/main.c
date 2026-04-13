#include <stdio.h>

int main(void)
{
    printf("Hello from GCC (MSYS2 UCRT64)!\n");
    printf("Compiler: %s\n", __VERSION__);
    printf("C Standard: %ld\n", __STDC_VERSION__);
    return 0;
}
