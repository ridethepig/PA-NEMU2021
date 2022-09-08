#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case EX_ecall_m: 
        if(c->GPR1 == -1)
          ev.event = EVENT_YIELD;
        else
          ev.event = EVENT_SYSCALL; 
      break; // only one cause for ecall, so EVENT_YIELD is set by a7 == -1
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context* context = kstack.end - sizeof(Context);
  memset(context, 0, sizeof(Context));
  // context->gpr[1] = (uintptr_t)entry;
  context->mepc = (uintptr_t)entry;
  context->mepc -= 4;
  context->mstatus = 0xa00001800;
  context->mcause = 0;
  context->gpr[10] = (uintptr_t)arg;
  context->gpr[2] = (uintptr_t)kstack.end;
  return context;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
