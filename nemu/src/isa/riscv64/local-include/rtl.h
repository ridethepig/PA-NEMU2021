#ifndef __RISCV64_RTL_H__
#define __RISCV64_RTL_H__

#include <rtl/rtl.h>
#include "reg.h"

// no isa-dependent rtl instructions

static inline def_rtl(csr2gpr, rtlreg_t *dest, int idx) {
    switch (idx)
    {
    case CSR_mcause:    *dest = cpu.mcause;  break;
    case CSR_mepc:      *dest = cpu.mepc;    break;
    case CSR_mstatus:   *dest = cpu.mstatus; break;
    case CSR_mtvec:     *dest = cpu.mtvec;   break;
    case CSR_satp:      *dest = cpu.satp;    break;
    case CSR_mscratch:  *dest = cpu.mscratch;break;
    default:
        panic("unsupported csr reg");
    }
}

static inline def_rtl(gpr2csr, rtlreg_t *src, int idx) {
    switch (idx)
    {
    case CSR_mcause:    cpu.mcause = *src;  break;
    case CSR_mepc:      cpu.mepc = *src;    break;
    case CSR_mstatus:   cpu.mstatus = *src; break;
    case CSR_mtvec:     cpu.mtvec = *src;   break;
    case CSR_satp:      cpu.satp = *src;    break;
    case CSR_mscratch:  cpu.mscratch = *src;break;
    default:
        panic("unsupported csr reg");
    }
}

#endif
