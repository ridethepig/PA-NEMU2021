#include <common.h>
#include "syscall.h"

static intptr_t syscall_write(int fd, void* buf, size_t count) {
  if (fd == 1 || fd == 2) {
    for (; count; -- count) {
      putch(*(char*)buf);
      buf ++;
    }
    return 0;
  }
  return 1;
}

static intptr_t syscall_brk(intptr_t addr) {
  return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
#ifdef CONFIG_STRACE
  printf("STRACE: [#%3ld]( %ld, %ld, %ld )\n", a[0], a[1], a[2], a[3]);
#endif
  switch (a[0]) {
    case SYS_yield:
      yield(); 
      c->GPRx = 0;
    break;
    case SYS_exit:
      halt(a[1]); c->GPRx = a[1];
    break;
    case SYS_write:
      c->GPRx = syscall_write(a[1], (void*)a[2], a[3]);
    break;
    case SYS_brk:
      c->GPRx = syscall_brk(a[1]);
    break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
#ifdef CONFIG_STRACE
  printf("        [#%3ld] -> %ld\n", a[0], c->GPRx);
#endif
}
