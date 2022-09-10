#include <nanos_memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  // pf has been inited in init_mm
  void *old_pf = pf;
  memset(pf, 0, nr_page * PGSIZE);
  pf += nr_page * PGSIZE;
  assert(pf < (void *)heap.end);
  // Log("new page allocated in %p", old);
  return old_pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE == 0); // PA assert this n
  return new_page(n / PGSIZE); // set zero: done in new_page
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
