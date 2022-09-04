#include <proc.h>
#include <elf.h>
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr, *ptr_ehdr = &ehdr;
  Elf_Phdr phdr, *ptr_phdr = &phdr;
  uint32_t i, phoff;
  ramdisk_read(ptr_ehdr, 0, sizeof(Elf_Ehdr));
  assert(*((uint32_t*)ptr_ehdr) == 0x464c457f);
  assert(ehdr.e_ident[EI_CLASS] == ELFCLASS64);
  assert(ehdr.e_ident[EI_DATA] == ELFDATA2LSB);
  assert(ehdr.e_machine == EM_RISCV);
  for (i = 0; i < ehdr.e_phnum; ++ i) {
    phoff = i * ehdr.e_phentsize + ehdr.e_phoff;
    ramdisk_read(ptr_phdr, phoff, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      void* ptr_segment = (void *)phdr.p_vaddr;
      ramdisk_read(ptr_segment, phdr.p_offset, phdr.p_filesz);
      memset(ptr_segment + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
    }
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

