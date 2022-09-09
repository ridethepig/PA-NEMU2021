#include <common.h>
#include <device.h>
#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  yield();
  int i;
  for (i = 0; i < len; ++ i) {
    putch(*((char*)buf + i));
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  yield();
  AM_INPUT_KEYBRD_T kbd_in = io_read(AM_INPUT_KEYBRD);
  if (kbd_in.keycode == AM_KEY_NONE)
    return 0;
  else {
    int _len = snprintf(buf, len, "%s %s\n", kbd_in.keydown ? "kd" : "ku", keyname[kbd_in.keycode]);
    // printf("%s %d %s %d\n", buf, len, keyname[kbd_in.keycode], kbd_in.keydown);
    return _len;
  }
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T fbctl = io_read(AM_GPU_CONFIG);
  int _len = snprintf(buf, len, "WIDTH : %d\n HEIGHT : %d\n", fbctl.width, fbctl.height);
  return _len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  yield();
  AM_GPU_CONFIG_T fbctl = io_read(AM_GPU_CONFIG);
  size_t max_offset = fbctl.height * fbctl.width * sizeof(uint32_t);
  if (offset + len > max_offset) {
    len = max_offset - offset;
  }
  AM_GPU_MEMCPY_T gpu_memcpy;
  gpu_memcpy.dest = offset;
  gpu_memcpy.size = len;
  gpu_memcpy.src = buf;
  ioe_write(AM_GPU_MEMCPY, &gpu_memcpy);
  return len; // len counts in BYTE!!!
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
