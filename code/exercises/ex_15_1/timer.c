#include "os.h"

extern void schedule(void);

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ
#define TIMER_INTERVAL_MS (TIMER_INTERVAL / 1000)

static uint32_t _tick = 0;

#define MAX_TIMER 10
static struct timer timer_list[MAX_TIMER];

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval)
{
	/* each CPU has a separate source of timer interrupts. */
	int id = r_mhartid();
	
	*(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void simple_timer_init()
{
	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		t->func = NULL; /* use .func to flag if the item is used */
		t->arg = NULL;
		t++;
	}

	/*
	 * On reset, mtime is cleared to zero, but the mtimecmp registers 
	 * are not reset. So we have to init the mtimecmp manually.
	 */
	timer_load(TIMER_INTERVAL_MS * 100);

	/* enable machine-mode timer interrupts. */
	w_mie(r_mie() | MIE_MTIE);
}

struct timer *simple_timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout)
{
	/* TBD: params should be checked more, but now we just simplify this */
	if (NULL == handler || 0 == timeout) {
		return NULL;
	}

	/* use lock to protect the shared timer_list between multiple tasks */
	spin_lock();

	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (NULL == t->func) {
			break;
		}
		t++;
	}
	if (NULL != t->func) {
		spin_unlock();
		return NULL;
	}

	t->func = handler;
	t->arg = arg;
	t->timeout_tick = _tick + timeout;

	spin_unlock();

	return t;
}

void simple_timer_delete(struct timer *timer)
{
	spin_lock();

	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (t == timer) {
			t->func = NULL;
			t->arg = NULL;
			break;
		}
		t++;
	}

	spin_unlock();
}

/* this routine should be called in interrupt context (interrupt is disabled) */
static inline void simple_timer_check()
{
	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (NULL != t->func) {
			if (_tick >= t->timeout_tick) {
				t->func(t->arg);

				/* once time, just delete it after timeout */
				t->func = NULL;
				t->arg = NULL;

				break;
			}
		}
		t++;
	}
}

#define USE_SKIP_LIST_TIMER
// #define USE_LIST_TIMER
// #define USE_SIMPLE_TIMER

extern void list_timer_init();
extern void list_timer_check();
extern struct timer *list_timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout);
extern void list_timer_delete(struct timer *timer);

extern void skip_list_timer_init();
extern void skip_list_timer_check();
extern timer *skip_list_timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout);
extern void skip_list_timer_delete(struct timer *timer);


void timer_init() {
#ifdef USE_SKIP_LIST_TIMER
	skip_list_timer_init();
#endif
#ifdef USE_LIST_TIMER
	list_timer_init();
#endif
#ifdef USE_SIMPLE_TIMER
	simple_timer_init();
#endif
}

void timer_check() {
#ifdef USE_SKIP_LIST_TIMER
	skip_list_timer_check();
#endif
#ifdef USE_LIST_TIMER
	list_timer_check();
#endif
#ifdef USE_SIMPLE_TIMER
	simple_timer_check();
#endif
}

struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout) {
#ifdef USE_SKIP_LIST_TIMER
	return skip_list_timer_create(handler, arg, timeout);
#endif
#ifdef USE_LIST_TIMER
	return list_timer_create(handler, arg, timeout);
#endif
#ifdef USE_SIMPLE_TIMER
	return simple_timer_create(handler, arg, timeout);
#endif
}

void timer_delete(struct timer *timer) {
#ifdef USE_SKIP_LIST_TIMER
	skip_list_timer_delete(timer);
#endif
#ifdef USE_LIST_TIMER
	list_timer_delete(timer);
#endif
#ifdef USE_SIMPLE_TIMER
	simple_timer_delete(timer);
#endif
}

void timer_handler() 
{
	_tick++;
	if (_tick % 100 == 0) {
		printf("tick: %d\n", _tick);
	}

	timer_check();

	timer_load(TIMER_INTERVAL_MS * 100);

	schedule();
}

uint32_t get_ticks()
{
	return _tick;
}
