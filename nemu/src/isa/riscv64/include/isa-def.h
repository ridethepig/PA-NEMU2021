#ifndef __ISA_RISCV64_H__
#define __ISA_RISCV64_H__

#include <common.h>

typedef union satp_t {
  struct {
    word_t ppn  :44;
    word_t asid :16;
    word_t mode : 4;
  };
  word_t val;
} satp_t;

typedef union pte_t {
  struct {
    word_t V        : 1;
    word_t R        : 1;
    word_t W        : 1;
    word_t X        : 1;
    word_t U        : 1;
    word_t G        : 1;
    word_t A        : 1;
    word_t D        : 1;
    word_t RSW      : 2;
    word_t ppn      :44;
    word_t reserved :10;
  };
  word_t val;
} pte_t;

typedef union sv39_vaddr_t
{
  struct {
    word_t offset   :12;
    word_t vpn0     : 9;
    word_t vpn1     : 9;
    word_t vpn2     : 9;
    word_t reserved :25;
  };
  word_t val;
}sv39_vaddr_t;

typedef union mstatus_t {
  struct {
    uint64_t wpri0    : 1;
    uint64_t sie      : 1;
    uint64_t wpri2    : 1;
    uint64_t mie      : 1;
    uint64_t wpri4    : 1;
    uint64_t spie     : 1;
    uint64_t ube      : 1;
    uint64_t mpie     : 1;
    uint64_t spp      : 1;
    uint64_t wpri10_9 : 2;
    uint64_t mpp      : 2;
    uint64_t fs       : 2;
    uint64_t xs       : 2;
    uint64_t mprv     : 1;
    uint64_t sum      : 1;
    uint64_t mxr      : 1;
    uint64_t tvm      : 1;
    uint64_t tw       : 1;
    uint64_t tsr      : 1;
    uint64_t wpri31_23: 9;
    uint64_t uxl      : 2;
    uint64_t sxl      : 2;
    uint64_t sbe      : 1;
    uint64_t mbe      : 1;
    uint64_t wpri62_38: 25;
    uint64_t sd       : 1;
  };
  word_t val;
} mstatus_t;

#define GET_MSTATUS(var) mstatus_t var; (var).val = cpu.mstatus
#define SET_MSTATUS(var) cpu.mstatus = (var).val
#define GET_SATP(var) satp_t var; (var).val = cpu.satp
#define SET_SATP(var) cpu.satp = (var).val

#define MSTATUS_MASK_MIE (1UL << 3)
#define MSTATUS_MASK_MPIE (1UL << 7)

#define MCAUSE_MASK_Int (1UL<<63)
#define MCAUSE_MASK_ExCode ~(1UL<<63)

#define CSR_satp      0x180
#define CSR_mstatus   0x300
#define CSR_mtvec     0x305
#define CSR_mscratch  0x340
#define CSR_mepc      0x341
#define CSR_mcause    0x342

typedef struct {
  union {
    uint64_t _64;
  } gpr[32];
  vaddr_t pc;
  // put new members at the end, or difftest will break down
  uint64_t mstatus;
  uint64_t mepc;
  uint64_t mtvec;
  uint64_t mcause;
  uint64_t mscratch;
  uint64_t satp;
  bool INTR;
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

#define  INT_soft_s     0x8000000000000001
#define  INT_soft_m     0x8000000000000003
#define  INT_timer_s    0x8000000000000005
#define  INT_timer_m    0x8000000000000007
#define  INT_external_s 0x8000000000000009
#define  INT_external_m 0x800000000000000b

#endif
