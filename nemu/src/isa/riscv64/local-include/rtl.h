#ifndef __RISCV64_RTL_H__
#define __RISCV64_RTL_H__

#include <rtl/rtl.h>
#include "reg.h"

// no isa-dependent rtl instructions

static inline def_rtl(csr2gpr, rtlreg_t *dest, int idx) {
    switch (idx)
    {
    case CSR_mcause:
    case CSR_mepc:
    case CSR_mstatus:
    case CSR_mtvec:
    case CSR_satp:
        break;
    default:
        panic("unsupported csr reg");
    }
    *dest = csr(idx);
}

static inline def_rtl(gpr2csr, rtlreg_t *src, int idx) {
    switch (idx)
    {
    case CSR_mcause:
    case CSR_mepc:
    case CSR_mstatus:
    case CSR_mtvec:
    case CSR_satp:
        break;
    default:
        panic("unsupported csr reg");
    }
    csr(idx) = *src;
}

#endif
