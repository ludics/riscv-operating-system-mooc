#include "../os.h"

static struct st_list *list_timer = NULL;

extern void timer_load(uint32_t interval);

void list_timer_init() {
  list_timer = list_init();
  timer_load(CLINT_TIMEBASE_FREQ / 10);
  w_mie(r_mie() | MIE_MTIE);
  printf("list_timer_init: list_timer: %x\n", list_timer);
  mm_print_blocks();
}

struct timer *list_timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout) {
  if (NULL == handler || 0 == timeout) {
    return NULL;
  }
  spin_lock();
  struct timer *t = (struct timer *)mm_malloc(sizeof(struct timer));
  t->func = handler;
  t->arg = arg;
  t->timeout_tick = get_ticks() + timeout;
  struct st_list_node *node = (struct st_list_node *)mm_malloc(sizeof(struct st_list_node));
  node->data = t;
  node->priority = t->timeout_tick;
  list_sort_insert(list_timer, node);
  spin_unlock();
  return t;
}


void list_timer_delete(struct timer *timer) {
  spin_lock();
  struct st_list_node *node = list_timer->head;
  while (node != list_timer->tail) {
    if (node->data == timer) {
      list_delete(list_timer, node);
      mm_free(node);
      mm_free(timer);
      break;
    }
    node = node->next;
  }
  spin_unlock();
}

void list_timer_check() {
  while (1) {
    struct st_list_node *node = list_cbegin(list_timer);
    if (node == NULL || node == list_timer->tail) {
      break;
    }
    struct timer *t = (struct timer *)node->data;
    if (t->timeout_tick > get_ticks()) {
      break;
    }
    struct st_list_node *mut_node = list_pop_front(list_timer);
    struct timer *mut_t = (struct timer *)mut_node->data;
    if (mut_t->func != NULL) {
      mut_t->func(mut_t->arg);
    }
    mm_free(mut_node);
    mm_free(mut_t);
    // printf("list_timer_check: list size: %d, ticks: %u\n", list_size(list_timer), get_ticks());
    // mm_print_blocks();
    break;
  }
}