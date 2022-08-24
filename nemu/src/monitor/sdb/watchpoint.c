#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *empty = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr = NULL;
    wp_pool[i].old_val = 0;
  }

  head = NULL;
  empty = wp_pool;
}

/**
 * @param expr: const char input
 * @return return the number of allocated watchpoint
*/
WP* wp_new(const char * expr_str, bool *success) {
  static int count = 0; // not re-use NO. to avoid confusion
  WP* newnode = empty;
  word_t expr_val;
  bool _success;
  if (empty == NULL) {
    printf("No available node in watchpoint pool\n");
    assert(0);
  }
  expr_val = expr(expr_str, &_success);
  if (!_success) {
    *success = false;
    return 0;
  }
  empty = empty->next;
  newnode->next = head;
  head = newnode;
  newnode->NO = count ++;
  newnode->expr = (char*)malloc(strlen(expr_str) + 1);
  strcpy(newnode->expr, expr_str);
  newnode->old_val = expr_val;
  *success = true;
  return newnode;
}

/**
 * @param NO: allocated NO for the designated watchpoint
 */

bool wp_free(int NO) {
  WP* ptr = head;
  WP* pre = NULL;
  bool found = false;
  while (ptr) {
    if (ptr->NO == NO) {
      found = true;
      break;
    } else {
      pre = ptr;
      ptr = ptr->next;
    }
  }
  if (found) {
    free(ptr->expr); 
    ptr->old_val = 0;
    // remember to free memory
    if (ptr != head) {
      pre->next = ptr->next;
      ptr->next = empty->next;
      empty->next = ptr->next;
    } else {
      head = head->next;
      ptr->next = empty->next;
      empty->next = ptr->next;
    }
  }
  return found;
}

void wp_print() {
  WP* ptr = head;
  if (ptr == NULL) {
    printf("No watchpoint\n");
  }
  else {
    printf("|\tNO\t|     Old Value      |\t Expr\n");
    while (ptr) {
      printf("|\t%d\t|%20ld|  %s\n", ptr->NO, ptr->old_val, ptr->expr);
      ptr = ptr->next;
    }
  }
}

bool wp_check() {
  WP* ptr = head;
  word_t new_val;
  bool success;
  bool is_diff = false;
  while (ptr) {
    new_val = expr(ptr->expr, &success);
    if (success) {
      if (new_val != ptr->old_val) {
        printf("Watchpoint NO %d triggered: \n %s : %ld -> %ld\n", ptr->NO, ptr->expr, ptr->old_val, new_val);
        ptr->old_val = new_val;
        is_diff = true;
      }
    } else {
      printf("Watchpoint NO %d : \'%s\' failed to evaluate\n Execution will pause\n", ptr->NO, ptr->expr);
      return true; // pause nemu, let user decide whether to delete this watch
    }
    ptr = ptr->next;
  }
  return is_diff;
}