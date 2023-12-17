#include "skip_list.h"
#include "../os.h"

#define MAX_LEVEL 8

void skip_list_node_init(st_skip_list_node *node, uint32_t v, uint32_t l,
                         void *data, st_skip_list_node *next) {
  node->priority = v;
  node->level = l;
  node->data = data;
  for (int i = 0; i <= l; i++) {
    node->forward[i] = next;
  }
  for (int i = l + 1; i <= MAX_LEVEL; i++) {
    node->forward[i] = (st_skip_list_node *)0;
  }
}

int random_level() {
  int level = 0;
  while (randx() % 2) {
    level++;
  }
  return level < MAX_LEVEL ? level : MAX_LEVEL;
}

st_skip_list *skip_list_init() {
  st_skip_list *list = (st_skip_list *)mm_malloc(sizeof(st_skip_list));
  list->head = (st_skip_list_node *)mm_malloc(sizeof(st_skip_list_node));
  list->tail = (st_skip_list_node *)mm_malloc(sizeof(st_skip_list_node));
  skip_list_node_init(list->tail, INVALID, 0, NULL, NULL);
  skip_list_node_init(list->head, INVALID, MAX_LEVEL, NULL, list->tail);
  list->level = 0;
  list->length = 0;
  return list;
}

void skip_list_destroy(st_skip_list *list) {
  if (list == NULL) {
    return;
  }
  st_skip_list_node *p = list->head;
  while (p != list->tail) {
    st_skip_list_node *next = p->forward[0];
    mm_free(p);
    p = next;
  }
  mm_free(list->tail);
  mm_free(list);
}

st_skip_list_node *skip_list_search(st_skip_list *list, uint32_t priority) {
  if (list == NULL) {
    return NULL;
  }
  st_skip_list_node *p = list->head;
  for (int i = list->level; i >= 0; i--) {
    while (p->forward[i]->priority < priority) {
      p = p->forward[i];
    }
  }
  p = p->forward[0];
  if (p->priority == priority) {
    return p;
  }
  return NULL;
}

void skip_list_insert(st_skip_list *list, uint32_t priority, void *data) {
  if (list == NULL) {
    return;
  }
  st_skip_list_node *update[MAX_LEVEL + 1];
  st_skip_list_node *p = list->head;
  for (int i = list->level; i >= 0; i--) {
    while (p->forward[i]->priority < priority) {
      p = p->forward[i];
    }
    update[i] = p;
  }
  p = p->forward[0];
  int new_level = random_level();
  if (new_level > list->level) {
    for (int i = list->level + 1; i <= new_level; i++) {
      update[i] = list->head;
    }
    list->level = new_level;
  }

  st_skip_list_node *new_node =
      (st_skip_list_node *)mm_malloc(sizeof(st_skip_list_node));
  skip_list_node_init(new_node, priority, new_level, data, NULL);
  for (int i = 0; i <= new_level; i++) {
    new_node->forward[i] = update[i]->forward[i];
    update[i]->forward[i] = new_node;
  }
  list->length += 1;
}

void skip_list_insert2(st_skip_list *list, st_skip_list_node *node) {
  if (list == NULL) {
    return;
  }
  st_skip_list_node *update[MAX_LEVEL + 1];
  st_skip_list_node *p = list->head;
  for (int i = list->level; i >= 0; i--) {
    while (p->forward[i]->priority < node->priority) {
      p = p->forward[i];
    }
    update[i] = p;
  }
  p = p->forward[0];
  int new_level = random_level();
  if (new_level > list->level) {
    for (int i = list->level + 1; i <= new_level; i++) {
      update[i] = list->head;
    }
    list->level = new_level;
  }
  skip_list_node_init(node, node->priority, new_level, node->data, NULL);
  for (int i = 0; i <= new_level; i++) {
    node->forward[i] = update[i]->forward[i];
    update[i]->forward[i] = node;
  }
  list->length += 1;
}

int skip_list_delete(st_skip_list *list, uint32_t priority, int free_node) {
  if (list == NULL) {
    return 0;
  }
  st_skip_list_node *update[MAX_LEVEL + 1];
  st_skip_list_node *p = list->head;
  for (int i = list->level; i >= 0; i--) {
    while (p->forward[i]->priority < priority) {
      p = p->forward[i];
    }
    update[i] = p;
  }
  p = p->forward[0];
  if (p->priority != priority) {
    return 0;
  }
  for (int i = 0; i <= list->level; i++) {
    if (update[i]->forward[i] != p) {
      break;
    }
    update[i]->forward[i] = p->forward[i];
  }
  if (free_node) {
    mm_free(p);
  }
  while (list->level > 0 && list->head->forward[list->level] == list->tail) {
    list->level -= 1;
  }
  list->length -= 1;
  return 1;
}

int skip_list_delete2(st_skip_list *list, st_skip_list_node *node, int free_node) {
  if (list == NULL) {
    return 0;
  }
  st_skip_list_node *update[MAX_LEVEL + 1];
  st_skip_list_node *p = list->head;
  for (int i = list->level; i >= 0; i--) {
    while (p->forward[i] != node) {
      p = p->forward[i];
    }
    update[i] = p;
  }
  p = p->forward[0];
  if (p != node) {
    return 0;
  }
  for (int i = 0; i <= list->level; i++) {
    if (update[i]->forward[i] != p) {
      break;
    }
    update[i]->forward[i] = p->forward[i];
  }
  if (free_node) {
    mm_free(p);
  }
  while (list->level > 0 && list->head->forward[list->level] == list->tail) {
    list->level -= 1;
  }
  list->length -= 1;
  return 1;
}

st_skip_list_node *skip_list_cbegin(st_skip_list *list) {
  if (list == NULL) {
    return NULL;
  }
  return list->head->forward[0];
}

st_skip_list_node *skip_list_pop_front(st_skip_list *list) {
  if (list == NULL) {
    return NULL;
  }
  st_skip_list_node *node = list->head->forward[0];
  if (node == list->tail) {
    return NULL;
  }
  skip_list_delete(list, node->priority, 0);

  return node;
}

int skip_list_empty(st_skip_list *list) {
  if (list == NULL) {
    return 1;
  }
  if (list->length == 0) {
    return 1;
  }
  return 0;
}

int skip_list_size(st_skip_list *list) {
  if (list == NULL) {
    return 0;
  }
  return list->length;
}

void skip_list_print(st_skip_list *list) {
  if (list == NULL) {
    return;
  }
  st_skip_list_node *p = list->head->forward[0];
  while (p != list->tail) {
    printf("%d ", p->priority);
    p = p->forward[0];
  }
  printf("\n");
}

void skip_list_print_level(st_skip_list *list) {
  if (list == NULL) {
    return;
  }
  for (int i = list->level; i >= 0; i--) {
    st_skip_list_node *p = list->head->forward[i];
    while (p != list->tail) {
      printf("%d ", p->priority);
      p = p->forward[i];
    }
    printf("\n");
  }
  printf("\n");
}

void test_skip_list_1() {
  st_skip_list *list = skip_list_init();
  for (int i = 10; i > 0; i--) {
    skip_list_insert(list, i, NULL);
  }
  skip_list_print(list);
  skip_list_delete(list, 5, 1);
  skip_list_print(list);
  skip_list_delete(list, 0, 1);
  skip_list_print(list);
  skip_list_delete(list, 9, 1);
  skip_list_print(list);
  skip_list_delete(list, 4, 1);
  skip_list_print(list);
  st_skip_list_node *node = skip_list_search(list, 6);
  if (node != NULL) {
    printf("find node: %d\n", node->priority);
  } else {
    printf("not find node\n");
  }
  skip_list_delete(list, 6, 1);
  skip_list_print(list);
  st_skip_list_node *node2 = skip_list_search(list, 6);
  if (node2 != NULL) {
    printf("find node: %d\n", node2->priority);
  } else {
    printf("not find node\n");
  }
  skip_list_destroy(list);
}

void test_skip_list_2() {
  st_skip_list *list = skip_list_init();
  for (int i = 0; i < 100; i++) {
    skip_list_insert(list, i, NULL);
  }
  skip_list_print(list);
  for (int i = 0; i < 100; i++) {
    skip_list_delete(list, i, 1);
  }
  skip_list_print(list);
  skip_list_destroy(list);
}

int opers[90] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 1, 1, 0, 2, 2,
                  2, 1, 2, 0, 0, 0, 1, 2, 0, 2, 1, 2, 2, 1, 1, 0, 1, 2,
                  1, 1, 2, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 2, 2,
                  0, 1, 2, 0, 0, 2, 0, 2, 1, 1, 2, 2, 1, 2, 0, 1, 2, 1,
                  2, 1, 1, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2};
int nums[90] = {16, 5,  14, 13, 0,  3,  12, 9,  12, 3,  6,  7,  0,  1,  10,
                5,  12, 7,  16, 7,  0,  9,  16, 3,  2,  17, 2,  17, 0,  9,
                14, 1,  6,  1,  16, 9,  10, 9,  2,  3,  16, 15, 12, 7,  4,
                3,  2,  1,  14, 13, 12, 3,  6,  17, 2,  3,  14, 11, 0,  13,
                2,  1,  10, 17, 0,  5,  8,  9,  8,  11, 10, 11, 10, 9,  8,
                15, 14, 1,  6,  17, 16, 13, 4,  5,  4,  17, 16, 7,  14, 1};
int expected[90] = {0,  0,  0,  0,  0, 0,  0,  0,  0,  1,  -1, 0,  1,  -1, -1,
                    0,  1,  1,  1,  1, -1, 0,  0,  0,  -1, -1, 0,  -1, -1, 1,
                    1,  -1, -1, 0,  1, 1,  -1, 1,  1,  0,  0,  -1, 1,  -1, 0,
                    1,  0,  1,  1,  0, 0,  0,  -1, -1, 0,  1,  -1, 0,  0,  1,
                    0,  -1, -1, -1, 1, 1,  -1, 1,  0,  1,  -1, -1, -1, 1,  1,
                    -1, -1, 0,  0,  0, 0,  1,  1,  1,  1,  1,  1,  -1, -1, 1};

void test_skip_list_3() {
  st_skip_list *list = skip_list_init();
  printf("list size: %d\n", sizeof(st_skip_list));
  printf("node size: %d\n", sizeof(st_skip_list_node));

  for (int i = 0; i < 90; i++) {
    // skip_list_print(list);
    skip_list_print_level(list);
    if (opers[i] == 0) {
      skip_list_insert(list, nums[i], NULL);
      if (expected[i] != 0) {
        printf("i: %d, opers: %d, nums: %d, expected: %d\n", i, opers[i],
               nums[i], expected[i]);
      }
    } else if (opers[i] == 1) {
      int ret = skip_list_delete(list, nums[i], 1);
      if (ret == 1 && expected[i] != 1) {
        printf("i: %d, opers: %d, nums: %d, expected: %d\n", i, opers[i],
               nums[i], expected[i]);
      } else if (ret == 0 && expected[i] != -1) {
        printf("i: %d, opers: %d, nums: %d, expected: %d\n", i, opers[i],
               nums[i], expected[i]);
      }
    } else if (opers[i] == 2) {
      st_skip_list_node *node = skip_list_search(list, nums[i]);
      if (node != NULL && expected[i] != 1) {
        printf("i: %d, opers: %d, nums: %d, expected: %d\n", i, opers[i],
               nums[i], expected[i]);
      } else if (node == NULL && expected[i] != -1) {
        printf("i: %d, opers: %d, nums: %d, expected: %d\n", i, opers[i],
               nums[i], expected[i]);
      }
    }
  }
}

static const uint32_t MAX_TIMER = 100000;

void test_skip_list_benchmark() {
  printf("test_skip_list_benchmark\n");
  printf("MAX_LEVEL: %d\n", MAX_LEVEL);
  // sort insert 10000 nodes, then pop front 10000 nodes
  st_skip_list *list = skip_list_init();
  uint32_t start_time = r_rdtime();
  srandx(0x12345678);
  printf("start time: %d\n", start_time);
  uint32_t end_time = r_rdtime();
  // 统一分配
  void *p_node = mm_malloc(MAX_TIMER * sizeof(st_skip_list_node));
  void *p_timer = mm_malloc(MAX_TIMER * sizeof(timer));
  mm_print_blocks();
  uint32_t node_idx = 0;
  uint32_t timer_idx = 0;
  for (int i = 0; i < MAX_TIMER; i++) {
    int timeout = randx() % 10000;
    st_skip_list_node *node = (st_skip_list_node *)(p_node + node_idx * sizeof(st_skip_list_node));
    timer *t = (timer *)(p_timer + timer_idx * sizeof(timer));
    node_idx++;
    timer_idx++;
    t->timeout_tick = timeout;
    t->func = NULL;
    t->arg = NULL;
    node->priority = timeout;
    node->data = t;
    skip_list_insert2(list, node);
    if (i % (MAX_TIMER / 10) == 0) {
      printf("list size: %d, ticks %u, level %d\n", skip_list_size(list), get_ticks(), list->level);
    }
  }
  // skip_list_print_level(list);
  printf("list size: %d, ticks %u\n", skip_list_size(list), get_ticks());
  end_time = r_rdtime();
  printf("end time: %u, cost_time: %u \n", end_time, end_time - start_time);
  for (int i = 0; i < MAX_TIMER; i++) {
    if (skip_list_empty(list)) {
      break;
    }
    st_skip_list_node *node = skip_list_pop_front(list);
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
      printf("list size: %d, ticks %u\n", skip_list_size(list), get_ticks());
    }
  }
  printf("list size: %d, ticks %u\n", skip_list_size(list), get_ticks());
  mm_free(p_node);
  mm_free(p_timer);
  mm_print_blocks();
  skip_list_destroy(list);
  mm_print_blocks();
  end_time = r_rdtime();
  printf("end time: %u, cost_time: %u \n", end_time, end_time - start_time);
}
