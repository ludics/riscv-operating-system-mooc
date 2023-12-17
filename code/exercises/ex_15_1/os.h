#ifndef __OS_H__
#define __OS_H__

#include "types.h"
#include "riscv.h"
#include "platform.h"
#include "timer_structs/list.h"
#include "timer_structs/skip_list.h"

#include <stddef.h>
#include <stdarg.h>

/* uart */
extern int uart_putc(char ch);
extern void uart_puts(char *s);
extern int uart_getc(void);

/* printf */
extern int  printf(const char* s, ...);
extern void panic(char *s);

/* memory management */
// only malloc/free can use these functions, don't use them directly, or else the system will crash
// extern void *page_alloc(int npages);
// extern void page_free(void *p);

/* memory alloc */
extern void *mm_malloc(size_t size);
extern void mm_free(void *ptr);
extern void mm_print_blocks();

/* random */
extern void srandx(uint32_t seed);
extern uint32_t randx();

/* task management */
struct context {
	/* ignore x0 */
	reg_t ra;
	reg_t sp;
	reg_t gp;
	reg_t tp;
	reg_t t0;
	reg_t t1;
	reg_t t2;
	reg_t s0;
	reg_t s1;
	reg_t a0;
	reg_t a1;
	reg_t a2;
	reg_t a3;
	reg_t a4;
	reg_t a5;
	reg_t a6;
	reg_t a7;
	reg_t s2;
	reg_t s3;
	reg_t s4;
	reg_t s5;
	reg_t s6;
	reg_t s7;
	reg_t s8;
	reg_t s9;
	reg_t s10;
	reg_t s11;
	reg_t t3;
	reg_t t4;
	reg_t t5;
	reg_t t6;
	// upon is trap frame

	// save the pc to run in next schedule cycle
	reg_t pc; // offset: 31 *4 = 124
};

extern int  task_create(void (*task)(void* param), void* param, uint8_t prio, uint32_t ts);
extern void task_delay(volatile int count);
extern void task_yield();
extern void task_exit();
extern void back_to_os();
extern void task_sleep(uint32_t ticks);

/* plic */
extern int plic_claim(void);
extern void plic_complete(int irq);

/* lock */
extern int spin_lock(void);
extern int spin_unlock(void);

/* software timer */
typedef struct timer {
	void (*func)(void *arg);
	void *arg;
	uint32_t timeout_tick;
} timer;
extern struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout);
extern void timer_delete(struct timer *timer);
extern uint32_t get_ticks();

#endif /* __OS_H__ */
