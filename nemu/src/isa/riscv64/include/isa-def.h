#ifndef __ISA_RISCV64_H__
#define __ISA_RISCV64_H__

#include <common.h>

typedef union satp_t {
  struct {
    uint64_t ppn  :44;
    uint64_t asid :16;
    uint64_t mode : 4;
  };
  uint64_t val;
} satp_t;

typedef union pte_t {
  struct {
    uint64_t V        : 1;
    uint64_t R        : 1;
    uint64_t W        : 1;
    uint64_t X        : 1;
    uint64_t U        : 1;
    uint64_t G        : 1;
    uint64_t A        : 1;
    uint64_t D        : 1;
    uint64_t RSW      : 2;
    uint64_t ppn      :44;
    uint64_t reserved :10;
  };
  uint64_t val;
} pte_t;

typedef union sv39_vaddr_t
{
  struct {
    uint64_t offset   :12;
    uint64_t vpn0     : 9;
    uint64_t vpn1     : 9;
    uint64_t vpn2     : 9;
    uint64_t reserved :25;
  };
  uint64_t val;
}sv39_vaddr_t;

typedef struct {
  union {
    uint64_t _64;
  } gpr[32];
  vaddr_t pc;
  uint64_t sr[4096]; // no matter it is implemented or not
  // put new members at the end, or difftest will break down
} riscv64_CPU_state;

// decode
typedef struct {
  union {
    struct
    {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t rd         : 5;
      uint32_t funct3     : 3;
      uint32_t rs1        : 5;
      uint32_t rs2        : 5;
      uint32_t funct7     : 7;
    } r;
    
    struct {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t rd         : 5;
      uint32_t funct3     : 3;
      uint32_t rs1        : 5;
      int32_t  simm11_0   :12;
    } i;

    struct {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t rd         : 5;
      uint32_t funct3     : 3;
      uint32_t rs1        : 5;
      uint32_t csr        :12;
    } csr;

    struct {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t imm4_0     : 5;
      uint32_t funct3     : 3;
      uint32_t rs1        : 5;
      uint32_t rs2        : 5;
      int32_t  simm11_5   : 7;
    } s;
    struct {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t imm11      : 1;
      uint32_t imm4_1     : 4;
      uint32_t funct3     : 3;
      uint32_t rs1        : 5;
      uint32_t rs2        : 5;
      uint32_t imm10_5    : 6;
      int32_t  simm12     : 1;
    } b;
    struct {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t rd         : 5;
      int32_t  simm31_12  :20;
    } u;
    struct {
      uint32_t opcode1_0  : 2;
      uint32_t opcode6_2  : 5;
      uint32_t rd         : 5;
      uint32_t imm19_12   : 8;
      uint32_t imm11      : 1;
      uint32_t imm10_1    : 10;
      int32_t  simm20     : 1;
    } j;
    uint32_t val;
  } instr;
} riscv64_ISADecodeInfo;

// #define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)
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

#define CSR_mstatus 0x300
#define CSR_mepc    0x341
#define CSR_mcause  0x342
#define CSR_mtvec   0x305
#define CSR_satp    0x180
#endif
