#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // int i;
  // int w = 400;  // TODO: get the correct width
  // int h = 300;  // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t wh_data = inl(VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = (wh_data >> 16) & 0xffff, .height = wh_data & 0xffff,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int i, j, pi, pj;
  uint32_t* pixels = ctl->pixels;
  uint32_t wh_data = inl(VGACTL_ADDR);
  uint32_t width = (wh_data >> 16) & 0xffff;
  for (i = ctl->y, pi = 0; pi < ctl->h; ++ i, ++pi) {
    for (j = ctl->x, pj = 0; pj < ctl->w; ++ j, ++pj) {
      int fb_offset = (i * width + j) * 4;
      int pixel_offset = pi* ctl->w + pj;
      outl(FB_ADDR + fb_offset, *(pixels + pixel_offset));
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}


void __am_gpu_memcpy(AM_GPU_MEMCPY_T *gpu_memcpy) {
  int i;
  const char* src = (char *)gpu_memcpy->src;
  for (i = 0; i < gpu_memcpy->size; ++ i) {
    outb(FB_ADDR + i + gpu_memcpy->dest, *(src + i));
  }
  outl(SYNC_ADDR, 1);
}