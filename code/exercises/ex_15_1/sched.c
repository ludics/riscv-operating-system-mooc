#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024
/*
 * In the standard RISC-V calling convention, the stack pointer sp
 * is always 16-byte aligned.
 */
uint8_t __attribute__((aligned(16))) task_stack[MAX_TASKS][STACK_SIZE];
struct context ctx_tasks[MAX_TASKS];

uint8_t __attribute__((aligned(16))) stack_os[STACK_SIZE];
struct context ctx_os;

uint8_t task_prios[MAX_TASKS];
uint8_t task_status[MAX_TASKS];

#define TASK_EMPTY 0
#define TASK_READY 1
#define TASK_RUNNING 2
#define TASK_BLOCKED 3
#define TASK_EXITED 4
#define TASK_SLEEPING 5

#define task_schedulable(i) (task_status[i] == TASK_READY || task_status[i] == TASK_RUNNING)

uint32_t task_ts[MAX_TASKS];

/*
 * _current is used to point to the context of current task
 */
static int _current = -1;
static int _cur_ts = 0;

extern void os_schedule();

void sched_init()
{
	/* init ctx_os */
	ctx_os.sp = (reg_t) &stack_os[STACK_SIZE];
	ctx_os.pc = (reg_t) os_schedule;

	w_mscratch((reg_t)&ctx_os);

	/* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);

	// init task status & priority
	for (int i = 0; i < MAX_TASKS; i++) {
		task_status[i] = TASK_EMPTY;
		task_prios[i] = 0xff;
		task_ts[i] = 0;
	}
}

/*
 * implment a simple cycle FIFO schedular
 */
void schedule()
{
	// 先扫描获取最高优先级
	int cur_prio = 0xff;
	for (int i = 0; i < MAX_TASKS; i++) {
		if (task_schedulable(i) && task_prios[i] < cur_prio) {
			cur_prio = task_prios[i];
		}
	}
	// 扫描获取最高优先级的任务
	int next_task_id = -1;
	for (int i = 0; i < MAX_TASKS; i++) {
		if (task_schedulable(i) && task_prios[i] == cur_prio) {
			if (i > _current) {
				next_task_id = i;
				break;
			}
		}
	}
	// 如果没有找到，从头开始找
	if (next_task_id == -1) {
		for (int i = 0; i < MAX_TASKS; i++) {
			if (task_schedulable(i) && task_prios[i] == cur_prio) {
				next_task_id = i;
				break;
			}
		}
	}
	if (next_task_id == -1) {
		// 没有可调度的任务
		// printf("no schedulable task!\n");
		switch_to(&ctx_os);
	}
	// 检查下个任务的优先级是否比当前任务高，如果高则切换
	// 如果优先级相同，则检查当前任务的时间片是否用完
	if (_current == -1 || task_status[_current] == TASK_EXITED) {
		_current = next_task_id;
		_cur_ts = 1;
	} else if (task_prios[next_task_id] < task_prios[_current] ||
			(task_prios[next_task_id] == task_prios[_current] &&
			 task_ts[_current] <= _cur_ts )) {
		_current = next_task_id;
		_cur_ts = 1;
	} else {
		_cur_ts++;
	}
	// 切换到下一个任务
	_current = next_task_id;
	struct context *next = &(ctx_tasks[_current]);
	task_status[_current] = TASK_RUNNING;
	switch_to(next);
}

/*
 * DESCRIPTION
 * 	Create a task.
 * 	- start_routin: task routine entry
 * RETURN VALUE
 * 	0: success
 * 	-1: if error occured
 */
int task_create(void (*start_routin)(void* param), void* param,
								uint8_t prio, uint32_t ts)
{
	int task_id = -1;
	for (int i = 0; i < MAX_TASKS; i++) {
		if (task_status[i] == TASK_EMPTY || task_status[i] == TASK_EXITED) {
			task_id = i;
			break;
		}
	}
	if (task_id == -1) {
		return -1;
	}
	task_status[task_id] = TASK_READY;
	task_prios[task_id] = prio;
	task_ts[task_id] = ts;
	// init stack
	ctx_tasks[task_id].sp = (reg_t) &task_stack[task_id][STACK_SIZE];
	ctx_tasks[task_id].pc = (reg_t) start_routin;
	// init context
	ctx_tasks[task_id].a0 = (reg_t) param;
	return 0;
}

/*
 * DESCRIPTION
 * 	task_yield()  causes the calling task to relinquish the CPU and a new 
 * 	task gets to run.
 */
void task_yield()
{
	task_status[_current] = TASK_READY;
	/* trigger a machine-level software interrupt */
	int id = r_mhartid();
	*(uint32_t*)CLINT_MSIP(id) = 1;
}

/*
 * DESCRIPTION
 * 	task_exit() causes the calling task to exit.
 */
void task_exit()
{
	task_status[_current] = TASK_EXITED;
	/* trigger a machine-level software interrupt */
	int id = r_mhartid();
	*(uint32_t*)CLINT_MSIP(id) = 1;
}

/*
 * a very rough implementaion, just to consume the cpu
 */
void task_delay(volatile int count)
{
	count *= 50000;
	while (count--);
}


// kernel task delay using wfi instruction
void k_task_delay(volatile int count)
{
	count *= 500;
	while (count--) {
		asm volatile("wfi");
	}
}

void task_wakeup(void *arg)
{
	int task_id = (int)arg;
	task_status[task_id] = TASK_READY;
	/* trigger a machine-level software interrupt */
	int id = r_mhartid();
	*(uint32_t*)CLINT_MSIP(id) = 1;
}

void task_sleep(uint32_t ticks)
{
	task_status[_current] = TASK_SLEEPING;
	struct timer *t = timer_create(task_wakeup, (void*)_current, ticks);
	if (NULL == t) {
		printf("task_sleep: timer_create failed!\n");
		return;
	}
	/* trigger a machine-level software interrupt */
	int id = r_mhartid();
	*(uint32_t*)CLINT_MSIP(id) = 1;
}
