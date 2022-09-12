#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);
Context* __am_irq_handle(Context *c) {
  // printf("enter irq handle, context=%p\n", c);
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case EX_ecall_m: 
        if(c->GPR1 == -1)
          ev.event = EVENT_YIELD;
        else
          ev.event = EVENT_SYSCALL; 
      break; // only one cause for ecall, so EVENT_YIELD is set by a7 == -1
      case INT_timer_m:
        ev.event = EVENT_IRQ_TIMER;
      break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    // printf("user_handled, context=%p\n", c);
    assert(c != NULL);
  }
  __am_switch(c);
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
  context->mepc = (uintptr_t)entry;
  context->mstatus = 0xa00001880; 
  // enable int after context switch, but not in trap
  // so set mpie not mie, or the next intr may trigger intr though in trap
  context->mcause = 0;
  context->GPRx = (uintptr_t)arg;
  context->next_priv = 0;
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
