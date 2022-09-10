#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
  satp_t satp;
  satp.val = cpu.sr[CSR_satp];
  switch (satp.mode)
  {
  case 0x0: 
    return MMU_DIRECT;
  case 0x8: // only implementy sv39 without large page
    return MMU_TRANSLATE;
  case 0x9:
  default:
    return MMU_FAIL;
  } 
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  satp_t satp; satp.val = cpu.sr[CSR_satp];
  sv39_vaddr_t sv39_vaddr; sv39_vaddr.val = vaddr;
  // Log("try translate 0x%08lX", vaddr);
  uintptr_t pte2_addr = (satp.ppn << 12) + sv39_vaddr.vpn2*sizeof(pte_t);
  pte_t pte2; pte2.val = paddr_read((paddr_t)pte2_addr, sizeof(word_t));
  if (pte2.V == 0 || (pte2.R == 0 && pte2.W == 1)) assert(0);
  // PA has no such exception: manual II/P76/2
  assert(!(pte2.R || pte2.W || pte2.X)); // No Large Pages in PA

  uintptr_t pte1_addr = (pte2.ppn << 12) + sv39_vaddr.vpn1*sizeof(pte_t);
  pte_t pte1; pte1.val= paddr_read((paddr_t)pte1_addr, sizeof(word_t));
  if (pte1.V == 0 || (pte1.R == 0 && pte1.W == 1)) assert(0);
  // PA has no such exception: manual II/P76/2
  assert(!(pte1.R || pte1.W || pte1.X)); // No Large Pages in PA

  uintptr_t pte0_addr = (pte1.ppn << 12) + sv39_vaddr.vpn0*sizeof(pte_t);
  pte_t pte0; pte0.val= paddr_read((paddr_t)pte0_addr, sizeof(word_t));
  if (pte0.V == 0 || (pte0.R == 0 && pte0.W == 1)) assert(0);
  // PA has no such exception: manual II/P76/2
  assert((pte0.R || pte0.X)); // Must be leaf
  if (type == MEM_TYPE_IFETCH) assert(pte0.X);
  else if (type == MEM_TYPE_READ) assert(pte0.R);
  else if (type == MEM_TYPE_WRITE) assert(pte0.W);
  else assert(0);
  // treat type, keep simple
  // Log("translated 0x%08lX to 0x%08X", vaddr, (paddr_t)((pte0.ppn << 12) + sv39_vaddr.offset));
  return (paddr_t)((pte0.ppn << 12) + sv39_vaddr.offset);
}
