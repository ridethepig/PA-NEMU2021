// src1 <=> rs1, src2 <=> zext(csr), dest <=> rd

def_EHelper(csrrw) {
    // TODO: privilege checking, but not necessary in PA
    rtl_csr2gpr(s, ddest, s->src2.imm);
    // read first, rd==$0 is handled in decode stage
    rtl_gpr2csr(s, dsrc1, s->src2.imm);
}

def_EHelper(csrrs) {
    rtl_csr2gpr(s, ddest, s->src2.imm);
    rtl_or(s, s0, ddest, dsrc1); // s->dest now stores csr old value
    rtl_gpr2csr(s, s0, s->src2.imm);
}

def_EHelper(csrrc) {
    rtl_csr2gpr(s, ddest, s->src2.imm);
    rtl_not(s, s0, dsrc1);
    rtl_or(s, s0, ddest, s0); // s->dest now stores csr old value
    // csr(idx) = (~rs1) & csr_old
    rtl_gpr2csr(s, s0, s->src2.imm);
}

def_EHelper(csrrwi) {
    // TODO: privilege checking, but not necessary in PA
    rtl_csr2gpr(s, ddest, s->src2.imm);
    // read first, rd==$0 is handled in decode stage
    rtl_li(s, s0, s->src1.imm);
    rtl_gpr2csr(s, s0, s->src2.imm);
}

def_EHelper(csrrsi) {
    rtl_csr2gpr(s, ddest, s->src2.imm);
    rtl_li(s, s0, s->src1.imm);
    rtl_or(s, s0, ddest, s0); // s->dest now stores csr old value
    rtl_gpr2csr(s, s0, s->src2.imm);
}

def_EHelper(csrrci) {
    rtl_csr2gpr(s, ddest, s->src2.imm);
    rtl_li(s, s0, s->src1.imm);
    rtl_not(s, s0, s0);
    rtl_or(s, s0, ddest, s0); // s->dest now stores csr old value
    // csr(idx) = (~rs1) & csr_old
    rtl_gpr2csr(s, s0, s->src2.imm);
}

def_EHelper(ecall) {
    rtl_j(s, isa_raise_intr(EX_ecall_m, s->pc));
}