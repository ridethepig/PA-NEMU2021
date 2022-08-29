#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  // i dont know what the pc here means
  if (ref_r->pc != cpu.pc) {
    Log("PC not match: ref = "FMT_WORD", nemu = "FMT_WORD, ref_r->pc, cpu.pc);
    return false;
  }
  int i;
  for (i = 0; i < 32; ++ i) {
    if (difftest_check_reg(reg_name(i, 8) , pc, ref_r->gpr[i]._64, gpr(i)) == false) {
      return false;
    }
  }
  return true;
}

void isa_difftest_attach() {
}
