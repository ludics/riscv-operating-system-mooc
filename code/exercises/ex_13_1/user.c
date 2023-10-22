#include "os.h"

#define DELAY 1000

void user_task0(void* param)
{
	uart_puts("Task 0: Created!\n");

	task_yield();
	uart_puts("Task 0: I'm back!\n");
	while (1) {
		uart_puts("Task 0: Running...\n");
		task_delay(DELAY);
	}
}

void user_task1(void* param)
{
	uart_puts("Task 1: Created!\n");
	while (1) {
		uart_puts("Task 1: Running...\n");
		task_delay(DELAY);
	}
}

void user_task(void* param)
{
	int id = (int) param;
	printf("Task %d: Created!\n", id);
	int iter_cnt = id / 1000 + 5;
	while (1) {
		printf("Task %d: Running...\n", id);
		task_delay(DELAY);
		if (iter_cnt-- == 0) {
			break;
		}
	}
	printf("Task %d: Finished!\n", id);
	task_exit();
}

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
	// task_create(user_task0, NULL, 255, 0);
	// task_create(user_task1, NULL, 255, 0);
	task_create(user_task, (void *)3000, 0, 0);
	task_create(user_task, (void *)8003, 3, 0);
	task_create(user_task, (void *)2005, 5, 0);
	task_create(user_task, (void *)5000, 0, 0);
	task_create(user_task, (void *)9004, 4, 0);
	task_create(user_task, (void *)6001, 1, 0);
	task_create(user_task, (void *)7002, 2, 0);
	task_create(user_task, (void *)4000, 0, 0);
}

