#include <isa.h>
#include <memory/paddr.h>
#include "sdb.h"
#include <elf.h>
#include <cpu/cpu.h>
void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ASNI_FMT("ON", ASNI_FG_GREEN), ASNI_FMT("OFF", ASNI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ASNI_FMT(str(__GUEST_ISA__), ASNI_FG_YELLOW ASNI_BG_RED));
  printf("For help, type \"help\"\n");
  // Log("Exercise: Please remove me in the source code and compile NEMU again.");
  // assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>
#ifdef CONFIG_FTRACE
// name str will be copied, so feel free to free it
static void functab_push(const char* name, word_t addr, word_t size) {
  functab_node* newnode = (functab_node*) malloc(sizeof(functab_node));
  newnode->addr = addr;
  newnode->addr_end = addr + size;
  newnode->name = (char*)malloc(strlen(name) + 1);
  strcpy(newnode->name, name);
  newnode->next = functab_head;
  functab_head = newnode;
}

static void functab_print() {
  functab_node* ptr = functab_head;
  if (functab_head == NULL) {
    printf("No Function in symbol table\n");
  }
  while(ptr) {
    printf("Function %s @ "FMT_WORD" - "FMT_WORD"\n", ptr->name, ptr->addr, ptr->addr_end);
    ptr = ptr->next;
  }
}
#endif
void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

static long load_img() {
  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static long load_elf() {
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  Log("The elf is %s, size = %ld", elf_file, size);
  fseek(fp, 0, SEEK_SET);
  void* elf_buf = malloc(size);
  int ret = fread(elf_buf, size, 1, fp);
  Assert(ret == 1, "ELF executable '%s' read failed!", elf_file);
  fclose(fp);

// ELF Parse
  const uint32_t elf_magic = 0x464c457f;
  Elf64_Ehdr *elf_ehdr = elf_buf;
  uint32_t *magic = elf_buf;
  Assert(*magic == elf_magic, "Not a elf file");
  Assert(elf_ehdr->e_ident[EI_CLASS] == ELFCLASS64, "Not a 64bit elf, RV64 IS NOT compatible with RV32");
  Assert(elf_ehdr->e_ident[EI_DATA] == ELFDATA2LSB, "Not little endian");
  Assert(elf_ehdr->e_machine == EM_RISCV, "Not RISCV target");
  Assert(elf_ehdr->e_entry == RESET_VECTOR, "No support for jump to non-RESET location");
// Program Load
  int i;
  size_t img_size = 0;
  for (i = 0; i < elf_ehdr->e_phnum; ++ i) {
    int phdr_off = i * elf_ehdr->e_phentsize + elf_ehdr->e_phoff;
    Elf64_Phdr *elf_phdr = elf_buf + phdr_off;
    Assert(phdr_off < size, "Program header out of file");
    Assert(elf_phdr->p_offset < size, "Segment out of file");
    if (elf_phdr->p_type != PT_LOAD) continue;
    // At present we dont have memory map, so just copy?
    void* segment_ptr = guest_to_host(elf_phdr->p_vaddr);
    memcpy(segment_ptr, elf_buf + elf_phdr->p_offset, elf_phdr->p_filesz);
    memset(segment_ptr + elf_phdr->p_filesz, 0, elf_phdr->p_memsz - elf_phdr->p_filesz);
    img_size += elf_phdr->p_memsz;
  }
#ifdef CONFIG_FTRACE
// Symbol table parse
  Elf64_Shdr *symtab_shdr = NULL;
  Elf64_Shdr *shstrtab_shdr = (elf_ehdr->e_shstrndx * elf_ehdr->e_shentsize + elf_ehdr->e_shoff) + elf_buf;
  Elf64_Shdr *strtab_shdr = NULL;
  char* shstrtab_ptr = elf_buf + shstrtab_shdr->sh_offset;
  for (i = 0; i < elf_ehdr->e_shnum; ++ i) {
    int shdr_off = i * elf_ehdr->e_shentsize + elf_ehdr->e_shoff;
    Elf64_Shdr *elf_shdr = elf_buf + shdr_off;
    if (elf_shdr->sh_type == SHT_SYMTAB) {
      symtab_shdr = elf_shdr;
    }
    else if (elf_shdr->sh_type == SHT_STRTAB) {
      if (strcmp(shstrtab_ptr + elf_shdr->sh_name, ".strtab") == 0) {
        strtab_shdr = elf_shdr;
      }
    }
  }
  if (symtab_shdr != NULL) {
    Assert(strtab_shdr, "SYMTAB without name ??");
    printf("Found SYMTAB section: %s\n", &shstrtab_ptr[symtab_shdr->sh_name]);
    char* strtab_ptr = elf_buf + strtab_shdr->sh_offset;
    for (i = 0; i < symtab_shdr->sh_size; i += symtab_shdr->sh_entsize) {
      //* i work as offset here
      Elf64_Sym* elf_sym = elf_buf + symtab_shdr->sh_offset + i;
      // ! some symbol is SECTION type, so name not stored in .strtab
      if (ELF64_ST_TYPE(elf_sym->st_info) == STT_FUNC) {
        // printf("Found FUNC symbol: %s\n", strtab_ptr + elf_sym->st_name);
        functab_push(strtab_ptr + elf_sym->st_name, elf_sym->st_value, elf_sym->st_size);
      }
    }
    functab_print();
  } else {
    Log("No SYMTAB found");
  }
#endif
  free(elf_buf);
  Log("Equivalent img_size = %lu", img_size);
  return img_size;
}

static long load_program() {
  if (img_file != NULL && elf_file != NULL) {
    printf(ASNI_FMT("both image and elf are given, load image by default\n", ASNI_FG_MAGENTA));
  }
  if (img_file != NULL) {
    return load_img();
  } else if (elf_file != NULL) {
    return load_elf();
  } else {
    Log("No image is given. Use the default build-in image.");
    return 4096;// built-in image size
  }
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"image"    , required_argument, NULL, 'i'},
    {"elf"      , required_argument, NULL, 'e'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:i:e:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'i': img_file = optarg; break;
      case 'e': elf_file = optarg; break;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */
  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_program();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();
  /* for expr.c debug
  // assert(isa_reg_str2val_set("s0", 0x80000000));
  // isa_reg_display();
  // printf("= %ld\n", expr("1 * 2 * *($s0 + 16) * (0x20 + $s0) - $s0 / 10 && 10 - 10 == 0", &expr_succ));
  // exit(0);
  */
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv32, "riscv32",
    MUXDEF(CONFIG_ISA_riscv64, "riscv64", "bad")))) "-pc-linux-gnu"
  ));

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
