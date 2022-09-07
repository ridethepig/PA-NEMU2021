#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static uint64_t _usec = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  uint64_t cur_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  cur_usec -= _usec;
  return cur_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", O_RDONLY);
  assert(fd);
  int _len = read(fd, buf, len);
  close(fd);
  return _len;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  if (*h == 0 && *w == 0) {
    canvas_h = screen_h;
    canvas_w = screen_w;
  }
  else if (*h <= screen_h && *w <= screen_w) {
    canvas_h = *h;
    canvas_w = *w;
  }
  else
    printf("canvas size must < screen size (%d, %d)\n", screen_w, screen_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  x += (screen_w - canvas_w) / 2;
  y += (screen_h - canvas_h) / 2;
  int fd = open("/dev/fb", O_WRONLY);
  assert(fd);
  size_t base_offset = (y * screen_w + x) * sizeof(uint32_t);
  size_t pixel_offset = 0;
  int j;
  for (j = 0; j < h; ++ j) {
    lseek(fd, base_offset, SEEK_SET);
    write(fd, pixels + pixel_offset, w * sizeof(uint32_t));
    pixel_offset += w;
    base_offset += screen_w * sizeof(uint32_t);
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
// aquire base timestamp
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  _usec = tv.tv_sec * 1000000 + tv.tv_usec;

// aquire screen size
  char buf[80], buf_kv[40];
  int fd = open("/proc/dispinfo", O_RDONLY);
  assert(fd);
  read(fd, buf, 80);
  char* tok0_k = strtok(buf, ":");
  char* tok0_v = strtok(NULL, "\n");
  char* tok1_k = strtok(NULL, ":");
  char* tok1_v = strtok(NULL, "\n");
  sscanf(tok0_k, "%s", buf_kv);
  if (strcmp(buf_kv, "WIDTH") == 0) {
    screen_w = atoi(tok0_v);
  } else if (strcmp(buf_kv, "HEIGHT") == 0){
    screen_h = atoi(tok0_v);
  } else assert(0);
  sscanf(tok1_k, "%s", buf_kv);
  if (strcmp(buf_kv, "WIDTH") == 0) {
    screen_w = atoi(tok1_v);
  } else if (strcmp(buf_kv, "HEIGHT") == 0){
    screen_h = atoi(tok1_v);
  } else assert(0);
  close(fd);

  return 0;
}

void NDL_Quit() {
}
