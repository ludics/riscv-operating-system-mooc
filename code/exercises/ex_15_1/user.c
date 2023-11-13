#include "os.h"

#define DELAY 4000

struct userdata {
	int counter;
	char *str;
};

/* Jack must be global */
struct userdata person = {0, "Jack"};

void timer_func(void *arg)
{
	if (NULL == arg)
		return;

	struct userdata *param = (struct userdata *)arg;

	param->counter++;
	printf("======> TIMEOUT: %s: %d\n", param->str, param->counter);
}

void user_task_test_array(void* param) {
	uart_puts("Task test array: Created!\n");
	if (NULL == param) {
		printf("param is NULL!\n");
		task_exit();
	}
	uint32_t cycle = r_cycle();
	printf("cycle: %d\n", cycle);
	srandx(cycle);
	int count = 0;
	int size_array = (int) param;
	uint32_t* array = (uint32_t*)mm_malloc(size_array * sizeof(uint32_t));
	for (int i = 0; i < size_array; i++) {
		array[i] = randx() % 10000;
		if (array[i] > 5000) {
			count++;
		}
	}
	// print array
	printf("array:\n");
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; ++j) {
			printf("%u ", array[i * 10 + j]);
		}
		printf("\n");
	}
	printf("count: %d\n", count);
	printf("\n");
	// while (1) {
	// 	uart_puts("Task test array: Running... \n");
	// 	task_delay(DELAY);
	// }
	mm_free(array);
	while (1) {
		uart_puts("Task test array: Running... \n");
		task_delay(DELAY);
	}
	task_exit();
}

extern void test_list_benchmark();
extern void test_array_benckmark();

void user_task_test_list(void* param) {
	uart_puts("Task test list: Created!\n");
	test_list_benchmark();
	// test_array_benckmark();
	task_exit();
}

struct st_func {
	void (*start_routin)(void* param);
	void* param;
};

void create_task_timer_func(void *arg) {
	if (NULL == arg) {
		return;
	}
	struct st_func *param = (struct st_func *)arg;
  task_create(param->start_routin, param->param, 0, 0);
}

void my_task_test_timer(void* param) {
	uart_puts("Task test timer: Created!\n");
	if (NULL == param) {
		printf("param is NULL!\n");
		task_exit();
	}
	struct st_func *arg = (struct st_func *)mm_malloc(sizeof(struct st_func));
	arg->start_routin = user_task_test_array;
	arg->param = param;
	struct timer *t1 = timer_create(create_task_timer_func, arg, 5);
	if (NULL == t1) {
		printf("timer_create() failed!\n");
	}
	task_exit();
}

void user_task0(void* param)
{
	uart_puts("Task 0: Created!\n");

	struct timer *t1 = list_timer_create(timer_func, &person, 30);
	if (NULL == t1) {
		printf("list_timer_create() failed!\n");
	}
	struct timer *t2 = list_timer_create(timer_func, &person, 50);
	if (NULL == t2) {
		printf("list_timer_create() failed!\n");
	}
	struct timer *t3 = list_timer_create(timer_func, &person, 100);
	if (NULL == t3) {
		printf("list_timer_create() failed!\n");
	}
	while (1) {
		uart_puts("Task 0: Running... \n");
		// task_delay(DELAY);
		task_sleep(40);
	}
}

void user_task1(void* param)
{
	uart_puts("Task 1: Created!\n");
	while (1) {
		uart_puts("Task 1: Running... \n");
		// task_delay(DELAY);
		task_sleep(4);
	}
}

// uint32_t array[10000];

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
	task_create(user_task0, 0, 0, 0);
	task_create(user_task1, 0, 0, 0);
	// task_create(user_task1);
	// task_create(user_task_test_list, (void*)5000, 0, 0);
}
