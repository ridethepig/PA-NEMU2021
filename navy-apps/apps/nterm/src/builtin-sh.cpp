#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  // char* const argv[] = {NULL};
  // char* const envp[] = {NULL};
  char* argv[20] = {0}; // allocate 20 temp argv, assume wont overflow
  int argc = 0;
  // printf("Exec:%s\n", cmd);
  char* cmd_buf = (char*)malloc(strlen(cmd) + 1);
  strcpy(cmd_buf, cmd);
  char * tok0 = strtok(cmd_buf, " \n");
  // printf("CMD:%s\n", tok0);
  if (strcmp(tok0, "exit") == 0) {
    SDL_Quit();
  } else if (strcmp(tok0, "echo") == 0) {
    sh_printf("%s", cmd_buf + strlen(tok0) + 1);
  } else {
    argv[argc++] = tok0;
    while (tok0 = strtok(NULL, " \n")) {
      argv[argc ++] = tok0;
    }
    // for (int i = 0; i < argc; ++ i) 
    //   printf("%s", argv[i]);puts("");
    // return;
    if (execvp(argv[0], argv) == -1){
      sh_printf("Error execute: %s\n", argv[0]);
    }
  }
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  setenv("PATH", "/bin", 0);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
