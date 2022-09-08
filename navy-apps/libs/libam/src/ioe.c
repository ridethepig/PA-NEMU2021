#include <am.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <NDL.h>

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};
static uint64_t start_time = 0;
void __navy_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  uptime->us = tv.tv_sec * 1000000 + tv.tv_usec - start_time;
}

void __navy_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}

#define KEYDOWN_MASK 0x8000
void __navy_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  char buf[60], buf_key[30];
  int len = NDL_PollEvent(buf, sizeof(buf));
  int i;
  kbd->keydown = 0;
  kbd->keycode = 0;
  if (len) {
    char * tok0 = strtok(buf, " ");
    char * tok1 = strtok(NULL, " ");
    // process keytype
    assert(tok0[0] == 'k');
    switch (tok0[1])
    {
    case 'd':
      kbd->keydown = 1;
    break;
    case 'u':
      kbd->keydown = 0;
    break;
    default:assert(0);
    }
    // process key string
    kbd->keycode = 0;
    sscanf(tok1, "%s", buf_key);
    for (i = 0; i < sizeof(keyname) / sizeof(char*); ++ i) {
      if (strcmp(buf_key, keyname[i]) == 0) {
        kbd->keycode = i;
        break;
      }
    }
  }
}
void __navy_gpu_config(AM_GPU_CONFIG_T *cfg){
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
    cfg->width = atoi(tok0_v);
  } else if (strcmp(buf_kv, "HEIGHT") == 0){
    cfg->height = atoi(tok0_v);
  } else assert(0);
  sscanf(tok1_k, "%s", buf_kv);
  if (strcmp(buf_kv, "WIDTH") == 0) {
    cfg->width = atoi(tok1_v);
  } else if (strcmp(buf_kv, "HEIGHT") == 0){
    cfg->height = atoi(tok1_v);
  } else assert(0);
  cfg->present = true;
  cfg->has_accel = false;
  cfg->vmemsz = 0;
}
void __navy_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int i, pi;
  uint32_t* pixels = ctl->pixels;
  AM_GPU_CONFIG_T cfg;
  __navy_gpu_config(&cfg);
  uint32_t width = cfg.width;
  int fd = open("/dev/fb", O_WRONLY);
  size_t base_offset = (ctl->y * width + ctl->x) * sizeof(uint32_t);
  size_t pixel_offset = 0;
  for (i = 0; i < ctl->h; ++ i) {
    int ret_seek = lseek(fd, base_offset, SEEK_SET);
    int ret_write = write(fd, pixels+pixel_offset, ctl->w * sizeof(uint32_t));
    // printf("(%d, %d, %s) ", ret_seek, ret_write, strerror(errno));
    pixel_offset += ctl->w;
    base_offset += width * sizeof(uint32_t);
  }
}
void __navy_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

static void __navy_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __navy_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __navy_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __navy_timer_config,
  [AM_TIMER_RTC   ] = __navy_timer_rtc,
  [AM_TIMER_UPTIME] = __navy_timer_uptime,
  [AM_INPUT_CONFIG] = __navy_input_config,
  [AM_INPUT_KEYBRD] = __navy_input_keybrd,
  [AM_GPU_CONFIG  ] = __navy_gpu_config,
  [AM_GPU_FBDRAW  ] = __navy_gpu_fbdraw,
  [AM_GPU_STATUS  ] = __navy_gpu_status,
  // [AM_GPU_MEMCPY  ] = __navy_gpu_memcpy,
  [AM_UART_CONFIG ] = __navy_uart_config,
  // [AM_AUDIO_CONFIG] = __navy_audio_config,
  // [AM_AUDIO_CTRL  ] = __navy_audio_ctrl,
  // [AM_AUDIO_STATUS] = __navy_audio_status,
  // [AM_AUDIO_PLAY  ] = __navy_audio_play,
};

static void fail(void *buf) { puts("access nonexist register"); assert(0); }

bool ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  start_time = tv.tv_sec * 1000000 + tv.tv_usec;
  return true;
}

void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
