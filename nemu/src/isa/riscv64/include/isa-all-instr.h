#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(auipc) \
                    f(addi) f(slti) f(sltiu) f(xori) f(ori) f(andi) f(slli) f(srli) f(srai) \
                    f(addiw) f(slliw) f(srliw) f(sraiw)\
                    f(ld) f(lw) f(lh) f(lb) f(lwu) f(lhu) f(lbu) \
                    f(sd) f(sw) f(sh) f(sb) \
                    f(jal) f(jalr) \
                    f(inv) \
                    f(nemu_trap)

def_all_EXEC_ID();
