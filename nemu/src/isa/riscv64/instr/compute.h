def_EHelper(auipc) {
  rtl_li(s, ddest, id_src1->imm + s->pc);
}

def_EHelper(lui) {
  rtl_li(s, ddest, id_src1->imm);
}

def_EHelper(addi) {
  rtl_addi(s, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(slti) {
  rtl_setrelopi(s, RELOP_LT, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(sltiu) {
  rtl_setrelopi(s, RELOP_LTU, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(xori) {
  rtl_xori(s, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(ori) {
  rtl_ori(s, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(andi) {
  rtl_andi(s, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(slli) {
  rtl_slli(s, ddest, id_src1->preg, id_src2->imm); 
  // actually dont have to care about funct6(maybe), shamt is masked out in c_sll
}

def_EHelper(srli) {
  rtl_srli(s, ddest, id_src1->preg, id_src2->imm); 
}

def_EHelper(srai) {
  rtl_srai(s, ddest, id_src1->preg, id_src2->imm); 
}

def_EHelper(addiw) {
  rtl_addiw(s, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(slliw) {
  rtl_slliw(s, ddest, id_src1->preg, id_src2->imm); 
  // actually dont have to care about funct7, shamt is masked out in c_sll
}

def_EHelper(srliw) {
  rtl_srliw(s, ddest, id_src1->preg, id_src2->imm); 
}

def_EHelper(sraiw) {
  rtl_sraiw(s, ddest, id_src1->preg, id_src2->imm); 
}

def_EHelper(add) {
  rtl_add(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sub) {
  rtl_sub(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sll) {
  rtl_sll(s, ddest, dsrc1, dsrc2);
}

def_EHelper(slt) {
  rtl_setrelop(s, RELOP_LT, ddest, dsrc1, dsrc2);
}

def_EHelper(sltu) {
  rtl_setrelop(s, RELOP_LTU, ddest, dsrc1, dsrc2);
}

def_EHelper(xor) {
  rtl_xor(s, ddest, dsrc1, dsrc2);
}

def_EHelper(srl) {
  rtl_srl(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sra) {
  rtl_sra(s, ddest, dsrc1, dsrc2);
}

def_EHelper(or) {
  rtl_or(s, ddest, dsrc1, dsrc2);
}

def_EHelper(and) {
  rtl_and(s, ddest, dsrc1, dsrc2);
}

def_EHelper(addw) {
  rtl_addw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(subw) {
  rtl_subw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sllw) {
  rtl_sllw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(srlw) {
  rtl_srlw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sraw) {
  rtl_sraw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mul) {
  rtl_mulu_lo(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mulh) {
  rtl_muls_hi(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mulhu) {
  rtl_mulu_hi(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mulhsu) {
  rtl_mulsu_hi(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mulw) {
  rtl_mulw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(divs) {
  rtl_divs_q(s, ddest, dsrc1, dsrc2);
}

def_EHelper(divu) {
  rtl_divu_q(s, ddest, dsrc1, dsrc2);  
}

def_EHelper(rem) {
  rtl_divs_r(s, ddest, dsrc1, dsrc2);  
}

def_EHelper(remu) {
  rtl_divu_r(s, ddest, dsrc1, dsrc2);  
}

def_EHelper(divw) {
  rtl_divw(s, ddest, dsrc1, dsrc2);
}

def_EHelper(divuw) {
  rtl_divuw(s, ddest, dsrc1, dsrc2);  
}

def_EHelper(remw) {
  rtl_remw(s, ddest, dsrc1, dsrc2);  
}

def_EHelper(remuw) {
  rtl_remuw(s, ddest, dsrc1, dsrc2);  
}
