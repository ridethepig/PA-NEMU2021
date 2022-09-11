#ifndef ARCH_H__
#define ARCH_H__
#include <stdint.h>
struct Context {
  uintptr_t gpr[32];
  uintptr_t mcause;
  uintptr_t mstatus;
  uintptr_t mepc;
  void *pdir;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

enum {
  EX_instr_addr_misaligned = 0,
  EX_instr_access_fault,
  EX_illegal_instr,
  EX_breakpoint,
  EX_load_addr_misaligned,
  EX_load_access_fault,
  EX_store_addr_misaligned,
  EX_store_access_fault,
  EX_ecall_u,
  EX_ecall_s,
  EX_ecall_m = 11,
  EX_instr_page_fault,
  EX_load_page_fault,
  EX_store_page_fault = 15
};

#define  INT_soft_s     0x8000000000000001
#define  INT_soft_m     0x8000000000000003
#define  INT_timer_s    0x8000000000000005
#define  INT_timer_m    0x8000000000000007
#define  INT_external_s 0x8000000000000009
#define  INT_external_m 0x800000000000000b

#endif
