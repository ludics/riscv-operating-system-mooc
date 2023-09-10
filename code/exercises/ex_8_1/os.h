#ifndef __OS_H__
#define __OS_H__

#include "types.h"
#include "platform.h"

#include <stddef.h>
#include <stdarg.h>

/* uart */
extern int uart_putc(char ch);
extern void uart_puts(char *s);

/* printf */
extern int  printf(const char* s, ...);
extern void panic(char *s);

/* memory management */
// only malloc/free can use these functions, don't use them directly, or else the system will crash
// extern void *page_alloc(int npages);
// extern void page_free(void *p);

/* memory alloc */
extern void *mm_malloc(size_t size);
extern void mm_free(void *ptr);


#endif /* __OS_H__ */
