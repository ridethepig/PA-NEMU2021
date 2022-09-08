#include <am.h>
#include <stdlib.h>
Area heap = {};
// allocated nil heap
// cannot run oslab0-16xxxxxxx, it used unallocated heap
// microbench would ignore much of its benches due to heap
void putch(char ch) {
    putchar(ch);
}

void halt(int code) {
    exit(code);
}
