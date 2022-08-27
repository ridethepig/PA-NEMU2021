def_EHelper(auipc) {
  rtl_li(s, ddest, id_src1->imm + s->pc);
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
