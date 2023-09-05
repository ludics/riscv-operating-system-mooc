#include <stdio.h>

int main()
{
    unsigned int a = 0x87654321;
    unsigned int low = a & 0xffff;
    unsigned int high = a >> 16;
    printf("low: %x, high: %x\n", low, high);
    return 0;
}