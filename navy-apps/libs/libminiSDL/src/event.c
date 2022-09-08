#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
static uint8_t key_state[83] = {0};
static int parse_event(uint8_t *_key, uint8_t *_type) {
  char buf[60], buf_key[30];
  int len = NDL_PollEvent(buf, sizeof(buf));
  int i;
  uint8_t key = SDLK_NONE, type = SDL_KEYUP;
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
      // printf("key event: %d %d\n", *_key, *_type);
      return 1;
    }
    else {
      printf("unknown key: %s\n", tok1);
      return 0;
    }
  } else {
    if (_key) *_key = SDLK_NONE;
    if (_type) *_type = SDL_KEYUP;
    return 0;
  }
}

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  uint8_t key, type;
  int ret = parse_event(&key, &type);
  ev->type = type;
  ev->key.type = type;
  ev->key.keysym.sym = key;
  if (type == SDL_KEYDOWN)
    key_state[key] = 1;
  else if (ev->type == SDL_KEYUP)
    key_state[key] = 0;
  return ret;
}

int SDL_WaitEvent(SDL_Event *ev) {
  uint8_t key, type;
  while (parse_event(&key, &type) == 0);
  ev->type = type;
  ev->key.type = type;
  ev->key.keysym.sym = key;
  if (type == SDL_KEYDOWN)
    key_state[key] = 1;
  else if (ev->type == SDL_KEYUP)
    key_state[key] = 0;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  // never poll here, this only serve as count and get
  // or key_state wont be updated
  if (numkeys != NULL) {
    int _numkeys = 0;
    for (int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++)
    {
      if (key_state[i] == 1){
        _numkeys += 1;
      }
    }
    *numkeys = _numkeys;
  }
  return key_state;
}
