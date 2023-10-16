#include "os.h"

/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ

static uint32_t _tick = 0;

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval)
{
	/* each CPU has a separate source of timer interrupts. */
	int id = r_mhartid();
	
	*(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void timer_init()
{
	/*
	 * On reset, mtime is cleared to zero, but the mtimecmp registers 
	 * are not reset. So we have to init the mtimecmp manually.
	 */
	timer_load(TIMER_INTERVAL);

	/* enable machine-mode timer interrupts. */
	w_mie(r_mie() | MIE_MTIE);

	/* enable machine-mode global interrupts. */
	w_mstatus(r_mstatus() | MSTATUS_MIE);
}

char time_buf[8] = {'0', '0', ':', '0', '0', ':', '0', '0'};
char back_buf[8] = {'\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b'};

void timer_print()
{
	int sec = _tick % 60;
	int min = (_tick / 60) % 60;
	int hour = (_tick / 3600) % 24;
	time_buf[7] = '0' + sec % 10;
	time_buf[6] = '0' + sec / 10;
	time_buf[4] = '0' + min % 10;
	time_buf[3] = '0' + min / 10;
	time_buf[1] = '0' + hour % 10;
	time_buf[0] = '0' + hour / 10;
	if (_tick != 0) {
		printf("%s", back_buf);
	}
	printf("%s", time_buf);
}

void timer_handler() 
{
	timer_print();
	_tick++;
	//printf("tick: %d\n", _tick);
	timer_load(TIMER_INTERVAL);
}
