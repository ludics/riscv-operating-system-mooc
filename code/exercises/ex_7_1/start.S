  # size of each hart's stack is 1024 bytes
  .equ STACK_SIZE, 1024
  .equ MAXNUM_CPU, 8
  .equ UART0, 0x10000000
  .equ RHR, 0x00
  .equ THR, 0x00
  .equ DLL, 0x00
  .equ IER, 0x01
  .equ DLM, 0x01
  .equ FCR, 0x02
  .equ ISR, 0x02
  .equ LCR, 0x03
  .equ MCR, 0x04
  .equ LSR, 0x05
  .equ MSR, 0x06
  .equ SCR, 0x07

  .equ LSR_RX_READY, 0x01
  .equ LSR_TX_IDLE, 0x20

  .global _start
  .section .rodata
  hello_text:
    .asciz "Hello RVOS!\n"
  .text
_start:
  # park harts with id != 0
  csrr t0, mhartid    # read current hart id
  mv tp, t0      # keep CPU's hartid in its tp for later usage.
  bnez t0, park    # if we're not on the hart 0
          # we park the hart
  # Setup stacks, the stack grows from bottom to top, so we put the
  # stack pointer to the very end of the stack range.
  slli t0, t0, 10    # shift left the hart id by 1024
  la sp, stacks + STACK_SIZE  # set the initial stack pointer
          # to the end of the first stack space
  add sp, sp, t0    # move the current hart stack pointer
          # to its place in the stack space

  j start_kernel    # hart 0 jump to c

park:
  wfi
  j  park

# start_kernel start
start_kernel:
  addi sp, sp, -16
  sw ra, 12(sp)
  sw s0, 8(sp)
  addi s0, sp, 16

  call  uart_init
  la a0, hello_text
  call uart_puts

loop:
  j loop

  lw ra, 12(sp)
  lw s0, 8(sp)
  addi sp, sp, 16
  ret
# start_kernel end

# uart_init start
uart_init:
  addi sp, sp, -32
  sw s0, 28(sp)
  addi s0, sp, 32

  li t0, UART0 + IER
  sb zero, 0(t0) # disable interrupts

  li t0, UART0 + LCR
  lb t1, 0(t0)
  ori t1, t1, 0x80
  sb t1, 0(t0)

  li t0, UART0 + DLL
  li t1, 0x03
  sb t1, 0(t0)

  li t0, UART0 + DLM
  li t1, 0x00
  sb t1, 0(t0)

  li t0, UART0 + LCR
  li t1, 0x03
  sb t1, 0(t0)

  lw s0, 28(sp)
  addi sp, sp, 32
  ret
# uart_init end

# uart_putc start
uart_putc:
  addi sp, sp, -32
  sw s0, 28(sp)
  addi s0, sp, 32

uart_putc_loop:
  li t0, UART0 + LSR
  lb t1, 0(t0)
  andi t1, t1, LSR_TX_IDLE
  beqz t1, uart_putc_loop

  li t0, UART0 + THR
  sb a0, 0(t0)

  lw s0, 28(sp)
  addi sp, sp, 32
  ret
# uart_putc end

# uart_getc start
uart_getc:
  addi sp, sp, -32
  sw s0, 28(sp)
  addi s0, sp, 32

uart_getc_loop:
  li t0, UART0 + LSR
  lb t1, 0(t0)
  andi t1, t1, LSR_RX_READY
  beqz t1, uart_getc_loop

  li t0, UART0 + RHR
  lb a0, 0(t0)

  lw s0, 28(sp)
  addi sp, sp, 32
  ret
# uart_getc end

# uart_puts start
uart_puts:
  addi sp, sp, -32
  sw s0, 28(sp)
  addi s0, sp, 32

  mv a1, a0
uart_puts_loop:
  lb a0, 0(a1)
  beqz a0, uart_puts_end
  call uart_putc
  addi a1, a1, 1
  j uart_puts_loop

uart_puts_end:
  lw s0, 28(sp)
  addi sp, sp, 32
  ret
# uart_puts end

  # In the standard RISC-V calling convention, the stack pointer sp
  # is always 16-byte aligned.
.align 16
stacks:
  .skip STACK_SIZE * MAXNUM_CPU # allocate space for all the harts stacks

  .end        # End of file
