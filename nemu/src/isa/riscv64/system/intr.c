#include <isa.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.sr[CSR_mepc] = epc;
  cpu.sr[CSR_mcause] = NO;
#ifdef CONFIG_ETRACE
  Log("Ex Trace: EPC@" FMT_WORD" -> HNDL@" FMT_WORD ", CAUSE: "FMT_WORD"", epc, cpu.sr[CSR_mtvec], NO);
#endif
  return cpu.sr[CSR_mtvec];
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
