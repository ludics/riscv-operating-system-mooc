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
struct context ctx_os;

uint8_t task_priorities[MAX_TASKS];

#define TASK_EMPTY 0
#define TASK_READY 1
#define TASK_RUNNING 2
#define TASK_BLOCKED 3
#define TASK_EXITED 4

uint8_t task_status[MAX_TASKS];

#define task_schedulable(task_id) (task_status[task_id] == TASK_READY || task_status[task_id] == TASK_RUNNING)

/*
 * _top is used to mark the max available position of ctx_tasks
 * _current is used to point to the context of current task
 */
static int _current = -1;

static void w_mscratch(reg_t x)
{
	asm volatile("csrw mscratch, %0" : : "r" (x));
}

void sched_init()
{
	w_mscratch((reg_t) &ctx_os);
	for (int i = 0; i < MAX_TASKS; i++) {
		task_status[i] = TASK_EMPTY;
		task_priorities[i] = 0xff;
	}
}

void back_to_os()
{
	switch_to(&ctx_os);
}

/*
 * implment a simple cycle FIFO schedular
 */
void schedule()
{
	// 先扫描获取最高优先级
	int priority = 0xff;
	for (int i = 0; i < MAX_TASKS; i++) {
		if (task_schedulable(i) && task_priorities[i] < priority) {
			priority = task_priorities[i];
		}
	}
	// printf("priority: %d\n", priority);
	// 扫描获取最高优先级的任务id，可能有多个，round-robin
	int next_task_id = -1;
	for (int i = 0; i < MAX_TASKS; i++) {
		if (task_schedulable(i) && task_priorities[i] == priority) {
			if (i > _current) {
				next_task_id = i;
				break;
			}
		}
	}
	if (next_task_id == -1) {
		for (int i = 0; i < MAX_TASKS; i++) {
			if (task_schedulable(i) && task_priorities[i] == priority) {
				next_task_id = i;
				break;
			}
		}
	}
	if (next_task_id == -1) {
		printf("no schedulable task\n");
		task_delay(10000);
		return;
	}
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
int task_create(void (*start_routin)(void* param), void* param, uint8_t priority)
{
	int task_id = -1;
	for (int i = 0; i < MAX_TASKS; i++) {
		if (task_status[i] == TASK_EMPTY || task_status[i] == TASK_EXITED) {
			task_id = i;
			break;
		}
	}
	if (task_id != -1) {
		ctx_tasks[task_id].sp = (reg_t) &task_stack[task_id][STACK_SIZE];
		if (param != NULL) {
			ctx_tasks[task_id].a0 = (reg_t) param;
		}
		ctx_tasks[task_id].ra = (reg_t) start_routin;
		task_priorities[task_id] = priority;
		task_status[task_id] = TASK_READY;
		return 0;
	} else {
		return -1;
	}
}

/*
 * DESCRIPTION
 * 	task_exit()  causes the currently executing task to exit.
 */
void task_exit(void) {
	task_status[_current] = TASK_EXITED;
	back_to_os();
}

/*
 * DESCRIPTION
 * 	task_yield()  causes the calling task to relinquish the CPU and a new 
 * 	task gets to run.
 */
void task_yield()
{
	task_status[_current] = TASK_READY;
	back_to_os();
}

/*
 * a very rough implementaion, just to consume the cpu
 */
void task_delay(volatile int count)
{
	count *= 50000;
	while (count--);
}

