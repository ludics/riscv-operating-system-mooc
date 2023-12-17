#ifndef __SKIP_LIST_H__
#define __SKIP_LIST_H__

#include "../types.h"

// static const uint32_t MAX_LEVEL = 16;
static const uint32_t INVALID = UINT32_MAX;
#define MAX_LEVEL 8

/* skip list */
typedef struct st_skip_list_node {
  uint32_t priority; // sort by priority
  void *data;
  struct st_skip_list_node *forward[MAX_LEVEL + 1];
  uint32_t level;
} st_skip_list_node;

typedef struct st_skip_list {
  st_skip_list_node *head;
  st_skip_list_node *tail;
  uint32_t level;
  uint32_t length;
} st_skip_list;


void skip_list_node_init(st_skip_list_node *node, uint32_t v, uint32_t l,
                         void *data, st_skip_list_node *next);
st_skip_list *skip_list_init();
void skip_list_destroy(st_skip_list *list);
st_skip_list_node *skip_list_search(st_skip_list *list, uint32_t target);
void skip_list_insert(st_skip_list *list, uint32_t priority, void *data);
void skip_list_insert2(st_skip_list *list, st_skip_list_node *node);
int skip_list_delete(st_skip_list *list, uint32_t target, int free_node);
int skip_list_delete2(st_skip_list *list, st_skip_list_node *node, int free_node);
st_skip_list_node *skip_list_cbegin(st_skip_list *list);
st_skip_list_node *skip_list_pop_front(st_skip_list *list);
int skip_list_empty(st_skip_list *list);
int skip_list_size(st_skip_list *list);
void skip_list_print(st_skip_list *list);
void skip_list_print_level(st_skip_list *list);

#endif /* __SKIP_LIST_H__ */