#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(auipc) f(lui)\
                    f(addi) f(slti) f(sltiu) f(xori) f(ori) f(andi) f(slli) f(srli) f(srai) \
                    f(addiw) f(slliw) f(srliw) f(sraiw)\
                    f(add) f(sub) f(slt) f(sltu) f(xor) f(or) f(and) f(sll) f(srl) f(sra) \
                    f(addw) f(subw) f(sllw) f(srlw) f(sraw)\
                    f(ld) f(lw) f(lh) f(lb) f(lwu) f(lhu) f(lbu) \
                    f(sd) f(sw) f(sh) f(sb) \
                    f(jal) f(jalr) \
                    f(beq) f(bne) f(blt) f(bge) f(bltu) f(bgeu) \
                    f(mul) f(mulh) f(mulhsu) f(mulhu) f(divs) f(divu) f(rem) f(remu) \
                    f(mulw) f(divw) f(divuw) f(remw) f(remuw) \
                    f(inv) \
                    f(nemu_trap)

def_all_EXEC_ID();
