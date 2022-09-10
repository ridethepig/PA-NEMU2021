#include <isa.h>
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) {
  int mmu_type = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
  paddr_t paddr;
  if (mmu_type == MMU_DIRECT)
    paddr = addr;
  else if (mmu_type == MMU_TRANSLATE) {
    paddr = isa_mmu_translate(addr, len, MEM_TYPE_IFETCH);
  } else panic("Cannot handle MMU_FAIL or MMU_DYNAMIC");
  // assert(paddr == addr);
  return paddr_read(paddr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  int mmu_type = isa_mmu_check(addr, len, MEM_TYPE_READ);
  paddr_t paddr;
  if (mmu_type == MMU_DIRECT)
    paddr = addr;
  else if (mmu_type == MMU_TRANSLATE) {
    paddr = isa_mmu_translate(addr, len, MEM_TYPE_READ);
  } else panic("Cannot handle MMU_FAIL or MMU_DYNAMIC");
  // assert(paddr == addr);
  return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  int mmu_type = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
  paddr_t paddr;
  if (mmu_type == MMU_DIRECT)
    paddr = addr;
  else if (mmu_type == MMU_TRANSLATE) {
    paddr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
  } else panic("Cannot handle MMU_FAIL or MMU_DYNAMIC");
  // assert(paddr == addr);
  paddr_write(paddr, len, data);
}
