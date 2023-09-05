int foo(int a, int b)
{
    int c;
    asm volatile(
        "mul %0, %1, %1\n\t"
        "mul t0, %2, %2\n\t"
        "add %0, %0, t0\n\t"
        : "=r"(c)
        : "r"(a), "r"(b)
        : "t0"
    );
    // c = a * a + b * b;
    return c;
}