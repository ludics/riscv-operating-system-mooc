    .data
array: .byte 'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd', '!', 0
len: .word 0

    .text
    .global _start
_start:
    la t0, array
    la t1, len
    li t2, 0
loop:
    lb a0, 0(t0)
    beqz a0, end
    addi t2, t2, 1
    addi t0, t0, 1
    j loop
end:
    sw t2, 0(t1)
stop:
    j stop
.end