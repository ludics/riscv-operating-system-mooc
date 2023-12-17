#include "os.h"

/*
 * Following functions SHOULD be called ONLY ONE time here,
 * so just declared here ONCE and NOT included in file os.h.
 */
extern void uart_init(void);
extern void page_init(void);
extern void sched_init(void);
extern void schedule(void);
extern void os_main(void);
extern void trap_init(void);
extern void plic_init(void);
extern void timer_init(void);
extern void mm_init(void);
// extern void mm_test(void);

extern void k_task_delay(uint32_t ticks);

void os_schedule(void) {
	// 内核调度
	while (1) {
		// uart_puts("os_schedule: Activate next task\n");
		int id = r_mhartid();
		*(uint32_t*)CLINT_MSIP(id) = 1;
		uart_puts("os_schedule: Back to OS\n");
		k_task_delay(1);
	}
}

void start_kernel(void)
{
	uart_init();
	uart_puts("Hello, RVOS!\n");

	page_init();

	mm_init();
	// mm_test();

	trap_init();

	plic_init();

	timer_init();

	sched_init();

	os_main();

	schedule();

	uart_puts("Would not go here!\n");
	while (1) {}; // stop here!
}

