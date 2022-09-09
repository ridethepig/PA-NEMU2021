#include <common.h>
#include <fs.h>
#include <sys/time.h>
#include "syscall.h"
#include <proc.h>
// do not use syscall.h for any actual purpose
// files.h and syscall.h are both symlink to navy-apps
#ifdef CONFIG_STRACE
static char* syscall_names[] = {
  "SYS_exit",   "SYS_yield",  "SYS_open",   "SYS_read", "SYS_write",  "SYS_kill",   "SYS_getpid", "SYS_close",
  "SYS_lseek",  "SYS_brk",    "SYS_fstat",  "SYS_time", "SYS_signal", "SYS_execve", "SYS_fork",   "SYS_link",
  "SYS_unlink", "SYS_wait",   "SYS_times",  "SYS_gettimeofday"
};
#endif
void context_uload(PCB* ptr_pcb, const char* filename, char* const argv[], char* const envp[]);
void switch_boot_pcb();

static inline intptr_t syscall_execve(const char *filename, char * const argv[], char *const envp[]) {
  int fd = fs_open(filename, 0, 0);
  if (fd == -1) return -2;
  context_uload(current, filename, argv, envp);
  switch_boot_pcb();
  yield();
  return 0;
}

static inline intptr_t syscall_brk(intptr_t addr) {
  return 0;
}

static inline intptr_t syscall_gettimeofday(struct timeval * tv, struct timezone * tz) {
  uint64_t uptime = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = uptime / 1000000;
  tv->tv_usec = uptime % 1000000; // according to man, usec ranges [0, 999999]
  if (tz) {
    tz->tz_dsttime = 0;
    tz->tz_minuteswest = 0;
  }
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
      halt(a[1]); 
      // naive_uload(NULL, "/bin/menu");
      c->GPRx = a[1];
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
      c->GPRx = fs_read(a[1], (void*)a[2], a[3]);
    break;
    case SYS_write:
      c->GPRx = fs_write(a[1], (void*)a[2], a[3]);
    break;
    case SYS_lseek:
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
    break;
    case SYS_gettimeofday:
      c->GPRx = syscall_gettimeofday((struct timeval*)a[1], (struct timezone*)a[2]);
    break;
    case SYS_execve:
      c->GPRx = syscall_execve((char *)a[1], (char * const*)a[2], (char * const*)a[3]);
    break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
#ifdef CONFIG_STRACE
  printf("        [%s] -> %ld\n", syscall_names[a[0]], c->GPRx);
#endif
}
