#ifndef __LIST_H__
#define __LIST_H__

#include "../types.h"

/* list */
typedef struct st_list_node {
	uint32_t priority; // sort by priority
	void *data;
	struct st_list_node *prev;
	struct st_list_node *next;
} st_list_node;

typedef struct st_list {
	st_list_node *head;
	st_list_node *tail;
	uint32_t size;
} st_list;

st_list *list_init();
void list_destroy(st_list *list);
st_list_node *list_pop_front(st_list *list);
void list_push_back(st_list *list, st_list_node *node);
void list_sort_insert(st_list *list, st_list_node *node);
st_list_node *list_cbegin(st_list *list);
int list_empty(st_list *list);
void list_delete(st_list *list, st_list_node *node);
int list_size(st_list *list);

void test_list_benchmark();

#endif /* __LIST_H__ */
