    .data
s: .word 0, 0

.macro set_struct st
    la t0, \st
    sw a0, 0(t0)
    sw a1, 4(t0)
.endm

.macro get_struct st
    la t0, \st
    lw a0, 0(t0)
    lw a1, 4(t0)
.endm

    .text
    .global _start
_start:
    li a0, 0x12345678 # a = 0x12345678
    li a1, 0x87654321 # b = 0x87654321
    set_struct s
    li a0, 0
    li a1, 0 
    get_struct s
    j _start
.end
