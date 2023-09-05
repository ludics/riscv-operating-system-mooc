    .data
array: .word 0x11111111, 0xffffffff

    .text
    .global _start
_start:
    la t0, array # load address of array into t0
    lw a0, 0(t0) # load first element of array into a0 (i)
    lw a1, 4(t0) # load second element of array into a1 (j)
    j _start     # Jump to _start
.end      # End of file
