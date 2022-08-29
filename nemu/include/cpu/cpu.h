#ifndef __CPU_CPU_H__
#define __CPU_CPU_H__

#include <common.h>

void cpu_exec(uint64_t n);

typedef struct functab_node
{
    struct functab_node * next;
    char* name;
    vaddr_t addr;
    vaddr_t addr_end;
} functab_node;

extern functab_node* functab_head;
#endif
