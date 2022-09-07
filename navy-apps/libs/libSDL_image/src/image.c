#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *fp;
  size_t file_size;
  char *buf;
  SDL_Surface* surface;
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("image '%s' not found\n", filename);
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buf = (char *)malloc(file_size);
  assert(fread(buf, file_size, 1, fp)); // make sure it reads
  surface = STBIMG_LoadFromMemory(buf, file_size);
  free(buf);
  return surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
