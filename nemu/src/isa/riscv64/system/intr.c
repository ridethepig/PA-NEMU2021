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
  SET_MSTATUS(mstatus);
  
#ifdef CONFIG_ETRACE
  Log("Ex Trace: EPC@" FMT_WORD" -> HNDL@" FMT_WORD ", CAUSE: "FMT_WORD"", epc, cpu.sr[CSR_mtvec], NO);
#endif
  return cpu.mtvec;
}

word_t isa_query_intr() {
  if (cpu.mstatus & MSTATUS_MASK_MIE) {
    cpu.INTR = 0;
    return INT_timer_m;
  }
  return INTR_EMPTY;
}
