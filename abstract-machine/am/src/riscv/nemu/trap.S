#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((32 + 4 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  (32 * XLEN)
#define OFFSET_STATUS (33 * XLEN)
#define OFFSET_EPC    (34 * XLEN)
#define OFFSET_NP     (35 * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:
// 把概念上的ksp映射到mscratch寄存器
// 在Context结构体中添加一个新的成员np, 把概念上的c->np映射到它
// 把概念上的c->sp映射到c->gpr[sp]
  csrrw sp, mscratch, sp  // exchange mscratch(ksp) and sp (now mscratch is orignal sp)
  bnez sp, save_context   // if ksp != 0, goto save_context
  csrrw sp, mscratch, sp  // if ksp == 0 : sp = ksp (restore original sp)
                          // exchange back it, we need a zero mscratch as KERNEL tag
save_context: 
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)
  
  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
  
  csrr t0, mscratch 
  STORE t0, OFFSET_NP(sp)   // just store mscratch into np, 
                            // if ksp == 0, then np is zero, means np = KERNEL
                            // if ksp != 0, then np is orignal sp, means np = USER
  csrw mscratch, zero       // ksp = 0, support CTE re-entry
  
  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle

  mv sp, a0
  LOAD t0, OFFSET_NP(sp)
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mscratch, t0
  csrw mstatus, t1
  csrw mepc, t2

  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE

  csrrw sp, mscratch, sp
  bnez sp, return         // if sp != 0, means np == USER, it's original sp, and mscratch is ksp, just return
  csrrw sp, mscratch, sp  // if sp == 0, means np == KERNEL, and mscratch is ksp,
                          // have to exchange, ksp have to be ZERO here

return:
  mret
