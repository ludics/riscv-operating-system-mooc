
    .text
    .global _start
_start:
    la sp, stack_end
    li a0, 3
    call sum_squares
    addi s0, a0, 0
    li a0, 4
    call sum_squares
    addi s1, a0, 0

stop:
    j stop

sum_squares:
    # prologue
    addi sp, sp, -16
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw ra, 12(sp)

    # body
    li s0, 0 # sum
    li s1, 1 # j
    addi s2, a0, 0 # i
sum_sq_loop:
    bgt s1, s2, sum_sq_end
    addi a0, s1, 0
    call square
    add s0, s0, a0
    addi s1, s1, 1
    j sum_sq_loop
sum_sq_end:
    addi a0, s0, 0
    # epilogue
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

square:
    # prologue
    addi sp, sp, -12
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)

    # body
    addi s0, a0, 0
    addi s1, a0, 0
    mul s2, s0, s1
    addi a0, s2, 0

    # epilogue
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    addi sp, sp, 12
    ret

stack_start:
    .rept 64
    .word 0
    .endr
stack_end:
    .end