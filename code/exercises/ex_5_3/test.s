    .text
    .global _start
_start:
    li a0, 1
    li a1, 2
    li a2, 3
    li a3, 4
    add a4, a0, a1
    add a5, a2, a3
    sub a6, a4, a5
    j _start
.end      # End of file
