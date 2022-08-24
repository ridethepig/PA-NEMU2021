#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char * expr;
  word_t old_val;
  /* TODO: Add more members if necessary */

} WP;

word_t expr(const char *e, bool *success);
WP* wp_new(const char* expr_str, bool* success);
bool wp_free(int NO);
void wp_print();
bool wp_check();

#endif
