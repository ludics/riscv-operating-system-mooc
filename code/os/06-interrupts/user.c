#include "os.h"

#define DELAY 1000

void user_task0(void)
{
	uart_puts("Task 0: Created!\n");
	while (1) {
		uart_puts("Task 0: Running...\n");
		task_delay(DELAY);
		task_yield();
	}
}

void user_task1(void)
{
	uart_puts("Task 1: Created!\n");
	reg_t status = r_mstatus();
	reg_t mie = r_mie();
	reg_t mip = r_mip();
	while (1) {
		// uart_puts("Task 1: Running...\n");
		printf("status = %x, mie = %x, mip = %x\n", status, mie, mip);
		task_delay(DELAY);
		task_yield();
	}
}

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
	// task_create(user_task0);
	task_create(user_task1);
}

