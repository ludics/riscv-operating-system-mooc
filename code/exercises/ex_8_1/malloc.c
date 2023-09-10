#include "os.h"

extern void *page_alloc(size_t n);
extern void page_free(void *ptr);

// | header | data | footer |
#define BLOCK_USED 1
#define BLOCK_PREV_USED 2
#define BLOCK_FLAG (BLOCK_USED | BLOCK_PREV_USED)

static inline void *_block_get_header(void *block_ptr) {
  return block_ptr - sizeof(uint32_t);
}

static inline int _block_is_used(void *block_ptr) {
  void *block_header = block_ptr - sizeof(uint32_t);
  return *(uint32_t *)block_header & BLOCK_USED;
}

static inline void _block_set_used(void *block_ptr) {
  void *block_header = block_ptr - sizeof(uint32_t);
  *(uint32_t *)block_header |= BLOCK_USED;
}

static inline void _block_set_unused(void *block_ptr) {
  void *block_header = block_ptr - sizeof(uint32_t);
  *(uint32_t *)block_header &= ~BLOCK_USED;
}

// header 去除最后一位后，就是 block 的大小，单位是 4 字节
static inline size_t _block_get_size(void *block_ptr) {
  void *block_header = block_ptr - sizeof(uint32_t);
  return *(uint32_t *)block_header & ~BLOCK_FLAG;
}

static inline void _block_set_size(void *block_ptr, size_t size) {
  void *block_header = block_ptr - sizeof(uint32_t);
  *(uint32_t *)block_header = size | (*(uint32_t *)block_header & BLOCK_FLAG);
}

static inline size_t _get_alloc_size(size_t size) {
  size_t alloc_size = size + sizeof(uint32_t) * 2; // header + footer
  if (alloc_size % 4 != 0) {
    alloc_size += 4 - alloc_size % 4;
  }
  return alloc_size;
}

static inline void *_block_get_footer(void *block_ptr) {
  return block_ptr + _block_get_size(block_ptr) - sizeof(uint32_t) * 2;
}

static inline void *_block_set_footer(void *block_ptr) {
  void *footer = _block_get_footer(block_ptr);
  void *header = _block_get_header(block_ptr);
  *(uint32_t *)footer = *(uint32_t *)header;
  return footer;
}

static inline void *_block_get_next(void *block_ptr) {
  return block_ptr + _block_get_size(block_ptr);
}

static inline void *_block_get_prev(void *block_ptr) {
  void *prev_footer = block_ptr - sizeof(uint32_t) * 2;
  size_t prev_block_size = *(uint32_t *)prev_footer & ~BLOCK_FLAG;
  return block_ptr - prev_block_size;
}

/*
  内部维护一个页，记录当前页已经分配的情况，每次分配都从这个页中分配
  如果这个页不够用了，用 page_alloc 分配一个新的页
*/

#define PAGE_SIZE 4096
void *_mm_start = NULL;
void *_mm_end = NULL;
void *_mm_start_block = NULL;
void *_mm_end_block = NULL;

// 向 page 申请 1 个 page，返回 page 的起始地址
void mm_init() {
  _mm_start = page_alloc(1);
  _mm_end = _mm_start + PAGE_SIZE; // _mm_end 为页的结束地址 + 4
  // _mm_start 为 header 的地址，_mm_start_block 为第一个 block
  // 的地址，_mm_end_block 为最后一个 block 的地址
  _mm_start_block = _mm_start + sizeof(uint32_t);
  _mm_end_block = _mm_end - sizeof(uint32_t);
  _block_set_size(_mm_start_block, PAGE_SIZE - sizeof(uint32_t) * 2);
  _block_set_unused(_mm_start_block);
  _block_set_footer(_mm_start_block);
  _block_set_size(_mm_end_block, sizeof(uint32_t) * 2);
  _block_set_used(_mm_end_block);
  _block_set_footer(_mm_end_block);
}

// 最多分配 64 M 内存，超过这个大小的分配失败
#define MAX_MEM_ALLOC 64 * 1024 * 1024
// 每个内存块有一个 header，记录这个内存块的大小，以及是否已经分配
// 每个内存块有一个 footer，与 header 内容相同，合并内存块时使用

void mm_print_blocks() {
  void *block_ptr = _mm_start_block;
  printf("-- start to print blocks --\n");
  while (block_ptr <= _mm_end_block) {
    void *last_block_ptr = block_ptr;
    printf("\tblock: %p, size: %d, used: %d\n", block_ptr,
           (int)_block_get_size(block_ptr), _block_is_used(block_ptr));
    block_ptr = _block_get_next(block_ptr);
    if (block_ptr == last_block_ptr) {
      printf("\tblock: %p, size: %d, used: %d\n", block_ptr,
             (int)_block_get_size(block_ptr), _block_is_used(block_ptr));
      break;
    }
  }
  printf("-- end to print blocks --\n");
}

void *mm_malloc(size_t size) {
  // printf("malloc size: %d\n", (int)size);
  size_t alloc_size = _get_alloc_size(size);
  if (alloc_size > MAX_MEM_ALLOC) {
    printf("malloc too large: %d\n", (int)alloc_size);
    return NULL;
  }

  void *block_ptr = _mm_start_block;
  // 遍历所有的 block，找到第一个满足大小的 block，first fit 策略
  int found = 0;
  while (block_ptr < _mm_end_block) {
    if (!_block_is_used(block_ptr) &&
        _block_get_size(block_ptr) >= alloc_size) {
      found = 1;
      break;
    }
    block_ptr = _block_get_next(block_ptr);
  }
  if (!found) {
    // printf("malloc failed: %d\n", (int)alloc_size);
    // return NULL;
    // 如果没有找到满足大小的 block，申请一个新的页
    void *cur_end = _mm_end;
    int num_of_new_alloc_pages = 0;
    while (!found) {
      void *new_page = page_alloc(1);
      num_of_new_alloc_pages++;
      if (new_page == NULL) {
        printf("malloc failed: %d, already alloc %d pages\n", (int)alloc_size,
               num_of_new_alloc_pages);
        // 找到 _mm_end_block 前一个 block，将这个 block 设置为最后一个 block
        void *last_block_ptr = _mm_start_block;
        while (last_block_ptr < _mm_end_block) {
          if (_block_get_next(last_block_ptr) == _mm_end_block) {
            break;
          }
          last_block_ptr = _block_get_next(last_block_ptr);
        }
        _mm_end_block = cur_end - sizeof(uint32_t);
        _block_set_size(_mm_end_block, sizeof(uint32_t) * 2);
        _block_set_used(_mm_end_block);
        _block_set_footer(_mm_end_block);
        if (last_block_ptr != _mm_end_block) {
          size_t last_block_size = _mm_end_block - last_block_ptr;
          _block_set_size(last_block_ptr, last_block_size);
          _block_set_footer(last_block_ptr);
        }
        // 回收新申请的页，恢复到 cur_end
        while (_mm_end > cur_end) {
          _mm_end -= PAGE_SIZE;
          page_free(_mm_end);
        }
        if (_mm_end_block != _mm_end - sizeof(uint32_t)) {
          panic("_mm_end_block != _mm_end - sizeof(uint32_t)");
        }
        return NULL;
      }
      if (new_page != _mm_end) {
        panic("new_page != _mm_end");
      }

      _mm_end += PAGE_SIZE;
      // 尝试与当前的最后一个 block 合并
      void *last_block_ptr = _block_get_prev(_mm_end_block);
      if (!_block_is_used(last_block_ptr)) {
        size_t last_block_size = _block_get_size(last_block_ptr);
        _block_set_size(last_block_ptr, last_block_size + PAGE_SIZE);
        _block_set_unused(last_block_ptr);
        _block_set_footer(last_block_ptr);
      } else {
        last_block_ptr = _mm_end_block;
        _block_set_size(last_block_ptr, PAGE_SIZE);
        _block_set_unused(last_block_ptr);
        _block_set_footer(last_block_ptr);
      }
      // 设置新的 _mm_end_block
      _mm_end_block = _mm_end - sizeof(uint32_t);
      _block_set_size(_mm_end_block, sizeof(uint32_t) * 2);
      _block_set_used(_mm_end_block);
      _block_set_footer(_mm_end_block);
      // printf("alloc a new page: %p\n", new_page);
      // 检查新分配的块是否满足要求
      if (!_block_is_used(last_block_ptr) &&
          _block_get_size(last_block_ptr) >= alloc_size) {
        found = 1;
        block_ptr = last_block_ptr;
        break;
      }
    }
    // printf("alloc %d pages\n", num_of_new_alloc_pages);
  }
  // 找到满足大小的 block，分配这个 block
  _block_set_used(block_ptr);
  size_t block_size = _block_get_size(block_ptr);
  if (block_size > alloc_size) {
    // 如果这个 block 大于需要的大小，分割这个 block
    _block_set_size(block_ptr, alloc_size);
    _block_set_used(block_ptr);
    _block_set_footer(block_ptr);
    void *next_block_ptr = _block_get_next(block_ptr);
    _block_set_size(next_block_ptr, block_size - alloc_size);
    _block_set_unused(next_block_ptr);
    _block_set_footer(next_block_ptr);
  }
  return block_ptr;
}

void mm_free(void *ptr) {
  // printf("free: %p\n", ptr);
  if (ptr == NULL) {
    return;
  }
  _block_set_unused(ptr);
  void *next_block_ptr = _block_get_next(ptr);
  size_t next_block_size = _block_get_size(next_block_ptr);
  int next_block_used = _block_is_used(next_block_ptr);
  // printf("next_block_ptr: %p, size: %d, used: %d\n", next_block_ptr,
  // (int)next_block_size, next_block_used);
  if (next_block_ptr != ptr && next_block_ptr <= _mm_end_block &&
      !next_block_used) {
    _block_set_size(ptr, _block_get_size(ptr) + next_block_size);
    _block_set_footer(ptr);
  }
  void *prev_block_ptr = _block_get_prev(ptr);
  size_t prev_block_size = _block_get_size(prev_block_ptr);
  int prev_block_used = _block_is_used(prev_block_ptr);
  // printf("prev_block_ptr: %p, size: %d, used: %d\n", prev_block_ptr,
  // (int)prev_block_size, prev_block_used);
  if (prev_block_ptr != ptr && prev_block_ptr >= _mm_start_block &&
      !prev_block_used) {
    _block_set_size(prev_block_ptr, _block_get_size(ptr) + prev_block_size);
    _block_set_footer(prev_block_ptr);
  }
}

struct test_struct {
  int a;
  int b;
  int c;
};

void mm_test() {
  printf("mm_test:\n");
  // test alloc too large
  mm_print_blocks();
  void *p = mm_malloc(MAX_MEM_ALLOC + 1);
  printf("malloc too large: %p\n", p);
  struct test_struct *test = mm_malloc(sizeof(struct test_struct));
  printf("test_struct: %p\n", test);
  test->a = 1;
  test->b = 2;
  test->c = 3;
  printf("test_struct: %d %d %d\n", test->a, test->b, test->c);
  mm_print_blocks();
  void *block_4096 = mm_malloc(4096);
  printf("block_4096: %p\n", block_4096);
  mm_print_blocks();
  void *test_2 = mm_malloc(sizeof(struct test_struct));
  printf("test_2: %p\n", test_2);
  mm_print_blocks();
  void *block_4000 = mm_malloc(4000);
  printf("block_4000: %p\n", block_4000);
  mm_print_blocks();
  void *block_4096x3 = mm_malloc(4096 * 3);
  printf("block_4096x3: %p\n", block_4096x3);
  mm_print_blocks();

  printf("\n start to test free\n");
  mm_free(test_2);
  printf("free test_2\n");
  mm_print_blocks();
  mm_free(block_4000);
  printf("free block_4000\n");
  mm_print_blocks();
  mm_free(block_4096);
  printf("free block_4096\n");
  mm_print_blocks();
  mm_free(block_4096x3);
  printf("free block_4096x3\n");
  mm_print_blocks();
  mm_free(test);
  printf("free test\n");
  mm_print_blocks();

  // test alloc too large
  printf("\n start to test alloc too large\n");
  void *block_32M = mm_malloc(32 * 1024 * 1024);
  printf("block_32M: %p\n", block_32M);
  mm_print_blocks();
  void *block_32M_2 = mm_malloc(32 * 1024 * 1024);
  printf("block_32M_2: %p\n", block_32M_2);
  mm_print_blocks();
  void *block_32M_3 = mm_malloc(32 * 1024 * 1024);
  printf("block_32M_3: %p\n", block_32M_3);
  mm_print_blocks();
  // make last used
  void *block_4056 = mm_malloc(4056);
  printf("block_4056: %p\n", block_4056);
  mm_print_blocks();
  void *block_32M_4 = mm_malloc(32 * 1024 * 1024);
  printf("block_32M_4: %p\n", block_32M_4);
  mm_print_blocks();

  // test free
  printf("\n start to test free\n");
  mm_free(block_32M_4);
  printf("free block_32M_4\n");
  mm_print_blocks();
  mm_free(block_32M_3);
  printf("free block_32M_3\n");
  mm_print_blocks();
  mm_free(block_32M_2);
  printf("free block_32M_2\n");
  mm_print_blocks();
  mm_free(block_32M);
  printf("free block_32M\n");
  mm_print_blocks();
  mm_free(block_4056);
  printf("free block_4056\n");
  mm_print_blocks();
}
