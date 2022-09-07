#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static int parse_event(uint8_t *_key, uint8_t *_type) {
  char buf[60], buf_key[30];
  int len = NDL_PollEvent(buf, sizeof(buf));
  int i;
  uint8_t key = 0, type = 0;
  if (len) {
    char * tok0 = strtok(buf, " ");
    char * tok1 = strtok(NULL, " ");
    // process keytype
    switch (tok0[0]) {
      case 'k': 
        switch (tok0[1])
        {
        case 'd':
          type = SDL_KEYDOWN;
        break;
        case 'u':
          type = SDL_KEYUP;
        break;
        default:assert(0);
        }
      break;
      default: assert(0);
    }
    // process key string
    sscanf(tok1, "%s", buf_key);
    for (i = 0; i < sizeof(keyname) / sizeof(char*); ++ i) {
      if (strcmp(buf_key, keyname[i]) == 0) {
        key = i;
        break;
      }
    }
    if (key) {
      *_key = key;
      *_type = type;
      return 1;
    }
    else {
      printf("unknown key: %s\n", tok1);
      return 0;
    }
  } else 
  return 0;
}

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return parse_event(&ev->key.keysym.sym, &ev->type);
}

int SDL_WaitEvent(SDL_Event *event) {
  while (parse_event(&event->key.keysym.sym, &event->type) == 0);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
