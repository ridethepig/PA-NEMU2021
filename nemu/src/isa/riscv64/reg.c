#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  int i;
  printf("| reg\t|         Hex        |          Dec         |\n");
  for (i = 0; i < 32; ++ i) {
    printf("| %s\t| 0x%016lX | %20ld |\n", regs[i], gpr(i), gpr(i));
  }
  printf("| pc\t| 0x%016lX |\n", cpu.pc);
}

/*
* Accept only
*/
word_t isa_reg_str2val(const char *s, bool *success) {
  int i;
  if (strcmp(s, "0") == 0 || strcmp(s, "zero") == 0) {
    *success = true;
    return 0;
  } // better special judge 0
  if (strcmp(s, "pc") == 0) {
    *success = true;
    return cpu.pc;
  }
  for (i = 0; i < 32; ++ i) {
    if (strcmp(s, regs[i]) == 0) {
      *success = true;
      return gpr(i);
    }
  }
  *success = false;
  return 0;
}

bool isa_reg_str2val_set(const char *s, word_t value) {
  int i;
  for (i = 1; i < 32; ++ i) {
    if (strcmp(s, regs[i]) == 0) {
      gpr(i) = value;
      return true;
    }
  }
  return false;
}