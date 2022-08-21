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

static int parse_int(const char* str, const bool isSigned, int * result) {
  int ans = 0;
  int sign = 1;
  int i = 0;
  if (isSigned) {
    if (str[i] == '-') sign = -1, i ++;
    else if (str[i] == '+') sign = 1, i++;
  }
  while(str[i] != '\0') {
    if (!isdigit(str[i])) {
      return 1;
    } else {
      ans = ans * 10 + str[i] - '0';
    }
    i += 1;
  }
  *result = sign * ans;
  return 0;
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
  { "x", "Scan memory", cmd_x}
  /* TODO: Add more commands */

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
  int n_steps;
  if (arg == NULL) {
    cpu_exec(1);
  } else {
    n_steps = 0;
    // while(arg[i] != '\0') {
    //   if (!isdigit(arg[i])) {
    //     printf("Only unsigned int value can be accepted as step number\n");
    //     return 0;
    //   } else {
    //     n_steps = n_steps * 10 + arg[i] - '0';
    //   }
    //   i += 1;
    // }
    if (parse_int(arg, false, &n_steps)) {
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
    printf("not implemented yet\n");
  }
  else {
    printf("info r - display register values\n info w - display watchpoint info\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg = strtok(NULL, " ");
  int num;
  word_t i;
  word_t data;
  word_t addr;
  if (arg == NULL) {
    printf("Usage: x N EXPR\n");
  } else {
    num = 0;
    if (parse_int(arg, true, &num)) {
      printf("Only int value can be accepted as scan size\n");
    } else {
      const word_t base = 0x80000000UL;
      if (num >= 0) {
        for (i = 0; i < num; ++ i) {
          addr = base + i * 4;
          data = vaddr_read(addr, 4);
          printf("0x%016lX : 0x%08lX\n", addr, data);
        }
      } else {
        for (i = num; i < 0; ++ i) {
          addr = base + i * 4;
          data = vaddr_read(addr, 4);
          printf("0x%016lX : 0x%08lX\n", addr, data);
        }
      }
    }
  }
  return 0;
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
