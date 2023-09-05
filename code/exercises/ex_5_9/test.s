
    .text
    .global _start
    .global foo
_start:
    la sp, stack_end
    li a0, 3
    li a1, 4
    call foo
    mv s0, a0

stop:
    j stop
nop

stack_start:
    .rept 32
    .word 0
    .endr
stack_end:
    .end