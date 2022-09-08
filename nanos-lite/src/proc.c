#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* ptr_pcb, void(*entry)(void*), void* arg) {
  Area kstack;
  kstack.start = ptr_pcb;
  kstack.end = &ptr_pcb->stack[sizeof(ptr_pcb->stack)];
  ptr_pcb->cp = kcontext(kstack, entry, arg);
  Log("Stack [%p, %p) Context: %p", ptr_pcb, kstack.end, ptr_pcb->cp);
  Log("Context Pos %p", &ptr_pcb->cp);
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, "fuckyou");
  context_kload(&pcb[1], hello_fun, "loveyou");
  switch_boot_pcb();
  // Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]); 
  // Log("schedule %p -> %p", prev, current->cp);
  return current->cp;
}
