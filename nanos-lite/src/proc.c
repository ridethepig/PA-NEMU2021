#include <proc.h>
#include <nanos_memory.h>
#include <device.h>
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
    if (j % 100000 == 0)
      Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    // Log("before yield: context=%p", pcb[0].cp);
#ifdef NO_TI
    yield();
#endif
  }
}

void context_kload(PCB* ptr_pcb, void(*entry)(void*), void* arg) {
  Area kstack;
  kstack.start = ptr_pcb;
  kstack.end = &ptr_pcb->stack[sizeof(ptr_pcb->stack)];
  ptr_pcb->cp = kcontext(kstack, entry, arg);
 #ifdef NO_TI
  ptr_pcb->cp->mstatus = 0xa00001800;  
#endif
 // Log("Stack [%p, %p) Context: %p Context.pdir %p", ptr_pcb, kstack.end, ptr_pcb->cp, ptr_pcb->cp->pdir);
  // Log("Context Pos %p", &ptr_pcb->cp);
}

void context_uload(PCB* ptr_pcb, const char* filename, char* const argv[], char* const envp[]) {
  protect(&ptr_pcb->as);
  uintptr_t ustack = (uintptr_t)new_page(8);
  uintptr_t ustack_mapped = (uintptr_t)ptr_pcb->as.area.end;
  for (int i = 0; i < 8; ++ i) {
    map(&ptr_pcb->as,
        (void*)(ptr_pcb->as.area.end - (8 - i) * PGSIZE),
        (void*)(ustack + i * PGSIZE),
        MMAP_READ | MMAP_WRITE);
    // Log("map 0x%8lx -> 0x%8lx", ptr_pcb->as.area.end - (8 - i) * PGSIZE, ustack + i * PGSIZE);
  }

//* Set argv, envp
  //* collect arg count
  int argv_count = 0;
  int envp_count = 0;
  if (argv) {
    while (argv[argv_count]) argv_count ++;
  }
  if (envp) {
    while (envp[envp_count]) envp_count ++;
  } 
  // char* _argv[20] = {0};
  // char* _envp[20] = {0};
  char** _argv = (char**)ustack;
  char** _envp = (char**)(ustack + (argv_count+1)*sizeof(char*));
  //* use ustack bottom as temporary buffer for new argv and envp
  
  // Log("argv_count:%d, envp_count:%d", argv_count, envp_count);
  // Log("envp: %p", envp[0]);
  //* copy strings
  ustack += 8 * PGSIZE; // put on the bottom of the stack

  for (int i = 0; i < envp_count; ++ i) {
    size_t len = strlen(envp[i]) + 1;
    ustack -= len;
    ustack_mapped -= len;
    strcpy((char*)ustack, envp[i]);
    _envp[i] = (char*)ustack_mapped;
  }

  for (int i = 0; i < argv_count; ++ i) {
    size_t len = strlen(argv[i]) + 1;
    ustack -= len;
    ustack_mapped -= len;
    strcpy((char*)ustack, argv[i]);
    _argv[i] = (char*)ustack_mapped;
  }
  //* copy argv table
  size_t envp_size = sizeof(char*) * (envp_count + 1); // there should be a null at the end 
  size_t argv_size = sizeof(char*) * (argv_count + 1);
  ustack -= envp_size;
  ustack_mapped -= envp_size;
  memcpy((void*) ustack, _envp, envp_size);
  ustack -= argv_size;
  ustack_mapped -= argv_size;
  memcpy((void*) ustack, _argv, argv_size);
  
  //* set argc
  ustack -= sizeof(uintptr_t);
  ustack_mapped -= sizeof(uintptr_t);
  *(uintptr_t *)ustack = argv_count;
  
  uintptr_t entry = loader(ptr_pcb, filename);
  
  Area kstack;
  kstack.start = ptr_pcb; // this is for PCB on stack, processed by kernel
  kstack.end = &ptr_pcb->stack[sizeof(ptr_pcb->stack)];
  ptr_pcb->cp = ucontext(&ptr_pcb->as, kstack, (void*)entry);
  ptr_pcb->cp->GPRx = ustack_mapped;
  // ptr_pcb->cp->GPRx = ustack;
  Log("updir %p sp: %p", ptr_pcb->as.ptr, ustack_mapped);
#ifdef NO_TI
  ptr_pcb->cp->mstatus = 0xa00001800;  
#endif
}

void init_proc() {
  // context_kload(&pcb[0], hello_fun, "lalala");
  char* test_argv[] = {NULL};
  char* test_envp[] = {NULL};
  fg_pcb = 1;
  context_uload(&pcb[0], "/bin/hello", test_argv, test_envp);
  context_uload(&pcb[1], "/bin/nterm", test_argv, test_envp);
  context_uload(&pcb[2], "/bin/nslider", test_argv, test_envp);
  context_uload(&pcb[3], "/bin/bird", test_argv, test_envp);
  Log("pcb = {%p, %p, %p, %p}", &pcb[0], &pcb[1], &pcb[2], &pcb[3]);
  Log("cp  = {%p, %p, %p, %p}", pcb[0].cp, pcb[1].cp, pcb[2].cp, pcb[3].cp);
  switch_boot_pcb();
  // Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  static int prio_count = 0;
  current->cp = prev;
  assert(1 <= fg_pcb); assert(fg_pcb <= 3);
  if (prio_count < 100) {
    prio_count ++;
    current = &pcb[fg_pcb];
  } else {
    prio_count = 0;
    current = &pcb[0];
  }
  // Log("schedule %p(updir %p) -> %p(updir %p)", prev, prev->pdir, current->cp, current->cp->pdir);
  return current->cp;
}
