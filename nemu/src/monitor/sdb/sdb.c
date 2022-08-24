#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <ctype.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int parse_int(const char* str, const bool isSigned, bool * success) {
  int ans = 0;
  int sign = 1;
  int i = 0;
  if (isSigned) {
    if (str[i] == '-') sign = -1, i ++;
    else if (str[i] == '+') sign = 1, i++;
  }
  while(str[i] != '\0') {
    if (!isdigit(str[i])) {
      *success = false;
      return 0;
    } else {
      ans = ans * 10 + str[i] - '0';
    }
    i += 1;
  }
  *success = true;
  return sign * ans;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}
static int cmd_q(char *args) {
  return -1;
}
static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single Instruction", cmd_si},
  { "info", "Display register or watchpoint information", cmd_info},
  { "x", "Scan memory", cmd_x},
  { "p", "Print expression", cmd_p},
  { "w", "Watch expression value, pause when value changed", cmd_w},
  { "d", "Delete watchpoint by its NO", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  bool success;
  int n_steps;
  if (arg == NULL) {
    cpu_exec(1);
  } else {
    n_steps = parse_int(arg, false, &success);
    // while(arg[i] != '\0') {
    //   if (!isdigit(arg[i])) {
    //     printf("Only unsigned int value can be accepted as step number\n");
    //     return 0;
    //   } else {
    //     n_steps = n_steps * 10 + arg[i] - '0';
    //   }
    //   i += 1;
    // }
    if (success) {
      printf("Only unsigned int value can be accepted as step number\n");
      return 0;
    }
    arg = strtok(NULL, " ");
    if (arg != NULL) {
      printf("Too many arguments. Ignoring...\n");
    }
    cpu_exec(n_steps);
  }
  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("info r - display register values\n info w - display watchpoint info\n");
  }
  else if (strcmp("r", arg) == 0){
    isa_reg_display();
  }
  else if (strcmp("w", arg) == 0) {
    wp_print();
  }
  else {
    printf("info r - display register values\n info w - display watchpoint info\n");
  }
  return 0;
}

// TODO: memory access range check
static int cmd_x(char *args) {
  char *arg = strtok(NULL, " ");
  char *expr_str = arg + strlen(arg) + 1;
  int num;
  word_t i;
  word_t data;
  word_t addr;
  word_t base;
  bool success;
  if (arg == NULL) {
    printf("Usage: x N EXPR\n");
  } else {
    num = parse_int(arg, true, &success);
    if (!success) {
      printf("Only int value can be accepted as scan size\n");
    } else {
      base = expr(expr_str, &success);
      if (success) {
        if (num >= 0) {
          for (i = 0; i < num; ++ i) {
            addr = base + i * 4;
            data = vaddr_read(addr, 4);
            printf("0x%016lX : 0x%08lX\n", addr, data);
          }
        } else {
          for (i = -num; i > 0; -- i) {
            addr = base - i * 4;
            data = vaddr_read(addr, 4);
            printf("0x%016lX : 0x%08lX\n", addr, data);
          }
        }
      }
      else {
        printf("Failed to eval address expr\n");
      }
    }
  }
  return 0;
}

static int cmd_p(char *args) {
  word_t val;
  bool success;
  assert(args != NULL);
  val = expr(args, &success);
  if (success)
    printf("%ld\n", val);
  else
    printf("Failed to eval expr\n");
  return 0;
}

static int cmd_w(char *args) {
#ifdef CONFIG_WATCHPOINT
  bool success;
  WP* wp;
  wp = wp_new(args, &success);
  if (!success) {
    printf("Failed to create watchpoint with expr \'%s\'\n", args);
    return 1;
  } else {
    printf("Created watchpoint NO %d, now \'%s\'=%ld\n", wp->NO, wp->expr, wp->old_val);
    return 0;
  }
#else
  printf("watchpoint function turned off\n");
#endif
}

static int cmd_d(char *args) {
#ifdef CONFIG_WATCHPOINT
  bool success;
  int NO;
  NO = parse_int(args, false, &success);
  if (!success) {
    printf("Usage: d N, N is the number of the watchpoint\n");
    return 1;
  }
  success = wp_free(NO);
  if (!success) {
    printf("No such watchpoint NO %d\n", NO);
    return 1;
  }
  printf("Removed watchpoint NO %d\n", NO);
  return 0;
#else
  printf("watchpoint function turned off\n");
#endif
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
