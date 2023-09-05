    .text
    .global _start
_start:
    li a0, 1
    li a1, 2
    li a2, 3
    add a3, a0, a1
    sub a4, a3, a2
    j _start
.end      # End of file
