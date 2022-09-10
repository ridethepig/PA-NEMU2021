#include <nanos_memory.h>
#include <proc.h>

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

/** The brk() system call handler. 
 * compare to max_brk, alloc and map
*/
int mm_brk(uintptr_t brk) {
  // printf("mm_brk brk: 0x%08lx; max_brk 0x%08lx;", brk, current->max_brk);
  current->max_brk = ROUNDUP(current->max_brk, PGSIZE); // max_brk and brk are open -- [,brk)
  if (brk > current->max_brk) {
    int page_count = ROUNDUP(brk - current->max_brk, PGSIZE) >> 12;
    uintptr_t pages_start = (uintptr_t)new_page(page_count);
    for (int i = 0; i < page_count; ++ i) {
      map(&current->as, 
          (void*)(current->max_brk + i * PGSIZE), 
          (void*)(pages_start + i * PGSIZE),
          MMAP_READ|MMAP_WRITE
          );
    }
    current->max_brk += page_count * PGSIZE;
    // printf("--brked-- ");
  }
  // printf("max_brk 0x%08lx\n", current->max_brk);
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
