def_EHelper(jal) {
    rtl_li(s, ddest, s->pc + 4);
    rtl_j(s, s->pc + id_src1->imm);
}

def_EHelper(jalr) {
    rtl_li(s, ddest, s->pc + 4);
    rtl_addi(s, s0, dsrc1, id_src2->imm);
    rtl_andi(s, s0, s0, 0xfffffffffffffffe);
    rtl_jr(s, s0);
}