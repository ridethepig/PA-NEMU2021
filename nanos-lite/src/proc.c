#include <proc.h>
#include <nanos_memory.h>
#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
uintptr_t loader(PCB *pcb, const char *filename);
void* new_page(size_t nr_page);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    if (j % 10000 == 0)
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

void context_uload(PCB* ptr_pcb, const char* filename, char* const argv[], char* const envp[]) {
  Area kstack;
  uintptr_t ustack = (uintptr_t)(new_page(8) + 8 * PGSIZE);
  kstack.start = ptr_pcb; // this is for PCB on stack, processed by kernel
  kstack.end = &ptr_pcb->stack[sizeof(ptr_pcb->stack)];

// Set argv, envp
  int argv_count = 0;
  int envp_count = 0;
  char* _argv[20] = {0};
  char* _envp[20] = {0 }; // tmp solution
  // collect arg count
  if (argv) {
    while (argv[argv_count]) argv_count ++;
  }
  if (envp) {
    while (envp[envp_count]) envp_count ++;
  }
  // Log("argv_count:%d, envp_count:%d", argv_count, envp_count);
  // Log("envp: %p", envp[0]);
  // copy strings
  for (int i = 0; i < envp_count; ++ i) {
    ustack -= strlen(envp[i]) + 1;
    strcpy((char*)ustack, envp[i]);
    _envp[i] = (char*)ustack;
  }

  for (int i = 0; i < argv_count; ++ i) {
    ustack -= strlen(argv[i]) + 1;
    strcpy((char*)ustack, argv[i]);
    _argv[i] = (char*)ustack;
  }
  // copy argv table
  size_t envp_size = sizeof(char*) * (envp_count + 1);
  size_t argv_size = sizeof(char*) * (argv_count + 1);
  ustack -= envp_size; // there should be a null at the end 
  memcpy((void*) ustack, _envp, envp_size);
  ustack -= argv_size;
  memcpy((void*) ustack, _argv, argv_size);
  // set argc
  ustack -= sizeof(uintptr_t);
  *(uintptr_t *)ustack = argv_count;
  // set stack pos
  uintptr_t entry = loader(ptr_pcb, filename);
  ptr_pcb->cp = ucontext(NULL, kstack, (void*)entry);
  ptr_pcb->cp->GPRx = ustack;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, "fuckyou");
  char* test_argv[] = {"/bin/exec-test", NULL};
  char* test_envp[] = {"LOVER=fucker", NULL};
  context_uload(&pcb[1], "/bin/nterm", test_argv, test_envp);
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
