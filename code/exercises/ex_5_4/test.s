    .text
    .global _start
_start:
    li a0, 0x87654321
    slli a1, a0, 16
    srli a1, a1, 16
    srli a2, a0, 16
    j _start
.end      # End of file