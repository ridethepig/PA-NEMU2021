#ifndef _NANOS_DEIVCE_H_
#define _NANOS_DEIVCE_H_
#include <common.h>
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
void init_device();

extern int fg_pcb;
#endif