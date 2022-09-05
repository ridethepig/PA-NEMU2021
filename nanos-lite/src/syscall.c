#include <common.h>
#include <fs.h>
#include "syscall.h"
// do not use syscall.h for any actual purpose
// files.h and syscall.h are both symlink to navy-apps
#ifdef CONFIG_STRACE
static char* syscall_names[] = {
  "SYS_exit",   "SYS_yield",  "SYS_open",   "SYS_read", "SYS_write",  "SYS_kill",   "SYS_getpid", "SYS_close",
  "SYS_lseek",  "SYS_brk",    "SYS_fstat",  "SYS_time", "SYS_signal", "SYS_execve", "SYS_fork",   "SYS_link",
  "SYS_unlink", "SYS_wait",   "SYS_times",  "SYS_gettimeofday"
};
#endif
static inline intptr_t syscall_write(int fd, void* buf, size_t count) {
  if (fd == 1 || fd == 2) {
    for (; count; -- count) {
      putch(*(char*)buf);
      buf ++;
    }
    return 0;
  } else {
    return fs_write(fd, buf, count);
  }
}

static inline intptr_t syscall_read(int fd, void* buf, size_t len) {
  if (fd <= 2) {
    return 0;
  } else {
    return fs_read(fd, buf, len);
  }
}

static inline intptr_t syscall_lseek(int fd, size_t offset, int whence) {
  if (fd <= 2) {
    return 0;
  } else {
    return fs_lseek(fd, offset, whence);
  }
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
    case SYS_brk:
      c->GPRx = syscall_brk(a[1]);
    break;
    case SYS_open:
      c->GPRx = fs_open((char *)a[1], a[2], a[3]);
    break;
    case SYS_close:
      c->GPRx = fs_close(a[1]);
    break;
    case SYS_read:
      c->GPRx = syscall_read(a[1], (void*)a[2], a[3]);
    break;
    case SYS_write:
      c->GPRx = syscall_write(a[1], (void*)a[2], a[3]);
    break;
    case SYS_lseek:
      c->GPRx = syscall_lseek(a[1], a[2], a[3]);
    break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
#ifdef CONFIG_STRACE
  printf("        [%s] -> %ld\n", syscall_names[a[0]], c->GPRx);
#endif
}
