#include "os.h"

#define DELAY 4000

#define USE_LOCK

#ifdef USE_LOCK
struct spinlock lock;
#endif

void user_task0(void)
{
	uart_puts("Task 0: Created!\n");
	while (1) {
#ifdef USE_LOCK
		// spin_lock();
		acquire(&lock);
#endif
		uart_puts("Task 0: Begin ... \n");
		for (int i = 0; i < 5; i++) {
			uart_puts("Task 0: Running... \n");
			task_delay(DELAY);
		}
		uart_puts("Task 0: End ... \n");
#ifdef USE_LOCK
		// spin_unlock();
		release(&lock);
#endif
	}
}

void user_task1(void)
{
	uart_puts("Task 1: Created!\n");
	while (1) {
		uart_puts("Task 1: Begin ... \n");
		for (int i = 0; i < 5; i++) {
			uart_puts("Task 1: Running... \n");
			task_delay(DELAY);
		}
		uart_puts("Task 1: End ... \n");
	}
}

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
#ifdef USE_LOCK
	initlock(&lock, "test");
#endif
	task_create(user_task0);
	task_create(user_task1);
}

