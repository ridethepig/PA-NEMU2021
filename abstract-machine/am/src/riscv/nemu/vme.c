#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define VPN2(va) (((uintptr_t)va >> 30) & 0x1ff)
#define VPN1(va) (((uintptr_t)va >> 21) & 0x1ff)
#define VPN0(va) (((uintptr_t)va >> 12) & 0x1ff)
#define PPN(pte) ((pte << 2) & 0x3ffffffffff000)

void map(AddrSpace *as, void *va, void *pa, int prot) {
  // ignore prot
  // now only sv39 3-level page table
  // assert(as->area.start <= va);
  // assert(as->area.end > va);
  uintptr_t* pte2 = (uintptr_t*)((uintptr_t)as->ptr + VPN2(va) * sizeof(uintptr_t));
  if ((*pte2 & 0x1) == 0) {
    // need to alloc a page
    uintptr_t newpage = (uintptr_t)pgalloc_usr(PGSIZE);
    *pte2 = (newpage >> 2) | 1; // set V = 1, ppn44 = newpage>>2
  }
  uintptr_t* pte1 = (uintptr_t*)(PPN(*pte2) + VPN1(va) * sizeof(uintptr_t));
  if ((*pte1 & 0x1) == 0) {
    uintptr_t newpage = (uintptr_t)pgalloc_usr(PGSIZE);
    *pte1 = (newpage >> 2) | 1; // set V = 1, ppn44 = newpage>>2
  }
  uintptr_t* pte0 = (uintptr_t*)(PPN(*pte1) + VPN0(va) * sizeof(uintptr_t));
  // if ((*pte0 & 0x1) == 0) {
  //   printf("new   va[%p]->pa[%p]\n", va, pa);
  // } else {
  //   printf("remap pa[%p]->pa[%p]\n", PPN(*pte0), pa);
  // }
  *pte0 = ((uintptr_t)pa >> 2) | 0xf; // XWRV = 1111, no protection in PA
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context* context = kstack.end - sizeof(Context);
  memset(context, 0, sizeof(Context));
  context->pdir = as->ptr;
  // context->gpr[1] = (uintptr_t)entry;
  context->mepc = (uintptr_t)entry;
  context->mstatus = 0xa00001880;
  context->mcause = 0;
  // user stack is set by os
  return context;
}
