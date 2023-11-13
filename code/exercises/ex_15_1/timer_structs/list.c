#include "../os.h"

#include "list.h"

st_list *list_init() {
	st_list *list = (st_list *)mm_malloc(sizeof(st_list));
	list->head = (st_list_node *)mm_malloc(sizeof(st_list_node));
	list->tail = (st_list_node *)mm_malloc(sizeof(st_list_node));
	list->head->prev = NULL;
	list->tail->next = NULL;
	list->head->next = list->tail;
	list->tail->prev = list->head;
	list->size = 0;
	return list;
}

void list_destroy(st_list *list) {
	if (list == NULL) {
		return;
	}
	while (!list_empty(list)) {
		st_list_node *node = list_pop_front(list);
		if (node == NULL) {
			break;
		}
		if (node->data != NULL) {
			mm_free(node->data);
		}
		mm_free(node);
	}
	mm_free(list->head);
	mm_free(list->tail);
	mm_free(list);
}

st_list_node *list_pop_front(st_list *list) {
	if (list->head == NULL) {
		return NULL;
	}
	if (list->head->next == list->tail) {
		return NULL;
	}
	st_list_node *node = list->head->next;
	list->head->next = node->next;
	node->next->prev = list->head;
	list->size--;
	return node;
}

void list_push_back(st_list *list, st_list_node *node) {
	if (list->tail == NULL) {
		return;
	}
	if (node == NULL) {
		return;
	}
	node->next = list->tail;
	node->prev = list->tail->prev;
	list->tail->prev->next = node;
	list->tail->prev = node;
	list->size++;
}

void list_sort_insert(st_list *list, st_list_node *node) {
	if (list->head == NULL || node == NULL) {
		return;
	}
	st_list_node *p = list->head->next;
	while (p != list->tail) {
		if (p->priority > node->priority) {
			break;
		}
		p = p->next;
	}
	node->next = p;
	node->prev = p->prev;
	p->prev->next = node;
	p->prev = node;
	list->size++;
}

st_list_node* list_cbegin(st_list *list) {
	if (list->head == NULL) {
		return NULL;
	}
	return list->head->next;
}

int list_empty(st_list *list) {
	if (list->head == NULL) {
		return 1;
	}
	if (list->head->next == list->tail) {
		return 1;
	}
	return 0;
}

int list_size(st_list *list) {
	if (list->head == NULL) {
		return 0;
	}
	return list->size;
}

// only remove node from list, not free node
void list_delete(st_list *list, st_list_node *node) {
	if (list->head == NULL || node == NULL) {
		return;
	}
	node->prev->next = node->next;
	node->next->prev = node->prev;
	list->size--;
}

const uint32_t MAX_TIMER = 100000;

void test_list_benchmark() {
	// sort insert 10000 nodes, then pop front 10000 nodes
	st_list *list = list_init();
	uint32_t start_time = r_rdtime();
	srandx(0x12345678);
	printf("start time: %d\n", start_time);
	uint32_t end_time = r_rdtime();
	// 统一分配
	void *p_node = mm_malloc(MAX_TIMER * sizeof(st_list_node));
	void *p_timer = mm_malloc(MAX_TIMER * sizeof(timer));
	mm_print_blocks();
	uint32_t node_idx = 0;
	uint32_t timer_idx = 0;
	for (int i = 0; i < MAX_TIMER; i++) {
		int timeout = randx() % 10000;
		st_list_node *node = (st_list_node *)(p_node + node_idx * sizeof(st_list_node));
		node_idx++;
		timer *t = (timer *)(p_timer + timer_idx * sizeof(timer));
		timer_idx++;
		// st_list_node *node = (st_list_node *)mm_malloc(sizeof(st_list_node));
		// timer *t = (timer *)mm_malloc(sizeof(timer));
		t->timeout_tick = timeout;
		t->func = NULL;
		t->arg = NULL;
		node->priority = timeout;
		node->data = t;
		list_sort_insert(list, node);
		if (i % (MAX_TIMER / 10) == 0) {
			printf("list size: %d, ticks %u\n", list_size(list), get_ticks());
		}
	}
	printf("list size: %d, ticks %u\n", list_size(list), get_ticks());
	end_time = r_rdtime();
	printf("end time: %u, cost_time: %u \n", end_time, end_time - start_time);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (list_empty(list)) {
			break;
		}
		st_list_node *node = list_pop_front(list);
		if (node == NULL) {
			printf("node is NULL!\n");
			break;
		}
		timer *t = (timer *)node->data;
		if (t == NULL) {
			printf("t is NULL!\n");
			break;
		}
		// mm_free(t);
		// mm_free(node);
		if (i % (MAX_TIMER / 10) == 0) {
			printf("list size: %d, ticks %u\n", list_size(list), get_ticks());
		}
	}
	printf("list size: %d, ticks %u\n", list_size(list), get_ticks());
	mm_free(p_node);
	mm_free(p_timer);
	mm_print_blocks();
	list_destroy(list);
	mm_print_blocks();
	end_time = r_rdtime();
	printf("end time: %u, cost_time: %u \n", end_time, end_time - start_time);
}

void void_func(void *arg) {
	printf("void_func: %d\n", (uint32_t)arg);
}


void test_array_benckmark() {
	uint32_t start_time = r_rdtime();
	srandx(0x12345678);
	void *array_timer = mm_malloc(MAX_TIMER * sizeof(timer));
	// uint32_t end_time = r_rdtime();
	printf("start time: %d\n", start_time);
	mm_print_blocks();
	// init timer
	for (int i = 0; i < MAX_TIMER; i++) {
		timer *t = (timer *)(array_timer + i * sizeof(timer));
		t->func = NULL;
		t->arg = NULL;
	}
	printf("init timer, ticks %u\n", get_ticks());
	for (int i = 0; i < MAX_TIMER; i++) {
		int timeout = randx() % 10000;
		for (int j = 0; j < MAX_TIMER; j++) {
			timer *t = (timer *)(array_timer + j * sizeof(timer));
			if (t->func == NULL) {
				t->timeout_tick = timeout;
				t->func = void_func;
				t->arg = (void *)timeout;
				break;
			}
		}
		if (i % (MAX_TIMER / 10) == 0) {
			printf("add timer cnt %d ticks %u\n", i, get_ticks());
		}
	}
	printf("after add timer, ticks %u\n", get_ticks());
	// check & delete timer
	for (int cur_time = 0; cur_time < 10000; cur_time++) {
		for (int j = 0; j < MAX_TIMER; j++) {
			timer *t = (timer *)(array_timer + j * sizeof(timer));
			if (t->func != NULL) {
				if (cur_time >= t->timeout_tick) {
					// t->func(t->arg);
					t->func = NULL;
					t->arg = NULL;
				}
			}
		}
		if (cur_time % (10000 / 10) == 0) {
			printf("check time %d ticks %u\n", cur_time, get_ticks());
		}
	}
	printf("after check timer, ticks %u\n", get_ticks());
}
