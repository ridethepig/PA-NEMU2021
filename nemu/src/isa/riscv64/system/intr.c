#include <isa.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.mepc = epc;
  cpu.mcause = NO;

  GET_MSTATUS(mstatus);
  mstatus.mpie = mstatus.mie;
  mstatus.mie = 0;
#ifdef CONFIG_ETRACE
  Log("ETrace: EPC@0x%08lX STATUS:"FMT_WORD"->"FMT_WORD", CAUSE:"FMT_WORD, epc, cpu.mstatus, mstatus.val, NO);
#endif
  SET_MSTATUS(mstatus);
  return cpu.mtvec;
}

word_t isa_query_intr() {
  if (cpu.INTR && (cpu.mstatus & MSTATUS_MASK_MIE)) {
    cpu.INTR = 0;
    return INT_timer_m;
  }
  return INTR_EMPTY;
}
