def_EHelper(jal) {
    rtl_j(s, s->pc + id_src1->imm);
    rtl_li(s, ddest, s->pc + 4);
}

def_EHelper(jalr) {
    rtl_addi(s, s0, dsrc1, id_src2->imm);
    rtl_andi(s, s0, s0, 0xfffffffffffffffe);
    rtl_jr(s, s0);
    rtl_li(s, ddest, s->pc + 4);
}

def_EHelper(beq) {
    rtl_jrelop(s, RELOP_EQ, dsrc1, ddest, s->pc + id_src2->imm);
    // in this framework, S/B type has src1 <=> rs1, src2 <=> imm, dest <=> rs2
}

def_EHelper(bne) {
    rtl_jrelop(s, RELOP_NE, dsrc1, ddest, s->pc + id_src2->imm);
}

def_EHelper(blt) {
    rtl_jrelop(s, RELOP_LT, dsrc1, ddest, s->pc + id_src2->imm);
}

def_EHelper(bge) {
    rtl_jrelop(s, RELOP_GE, dsrc1, ddest, s->pc + id_src2->imm);
}

def_EHelper(bltu) {
    rtl_jrelop(s, RELOP_LTU, dsrc1, ddest, s->pc + id_src2->imm);
}

def_EHelper(bgeu) {
    rtl_jrelop(s, RELOP_GEU, dsrc1, ddest, s->pc + id_src2->imm);
}