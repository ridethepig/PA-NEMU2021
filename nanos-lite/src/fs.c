#include <fs.h>
#include <device.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t fs_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_EVENTS]   = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // initialize the size of /dev/fb
  AM_GPU_CONFIG_T fbctl = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].disk_offset = 0;
  file_table[FD_FB].size = fbctl.width * fbctl.height * sizeof(uint32_t);
}

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);


int fs_open(const char* pathname, int flags, int mode) {
  int i;
  for (i = 0; i < sizeof(file_table) / sizeof(Finfo); ++ i) {
    if (file_table[i].name && strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].fs_offset = file_table[i].disk_offset;
      return i; // simply assign file_table index to fd
    }
  }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  if (file_table[fd].read) {
    len = file_table[fd].read(buf, file_table[fd].fs_offset, len);
    file_table[fd].fs_offset += len;
    return len;
  }
  // fallback to ramdisk read
  if (file_table[fd].fs_offset - file_table[fd].disk_offset + len > file_table[fd].size) {
    len = file_table[fd].size + file_table[fd].disk_offset - file_table[fd].fs_offset;
  }
  Log("file_table[fd].fs_offset: %d, disk_off: %d, size: %d, len: %d", file_table[fd].fs_offset, file_table[fd].disk_offset,file_table[fd].size, len);
  ramdisk_read(buf, file_table[fd].fs_offset, len);
  file_table[fd].fs_offset += len;
  return len;
}

size_t fs_write(int fd, void *buf, size_t len) {
  if (file_table[fd].write) {
    len = file_table[fd].write(buf, file_table[fd].fs_offset,len);
    file_table[fd].fs_offset += len;
    return len;
  }
  if (file_table[fd].fs_offset - file_table[fd].disk_offset + len > file_table[fd].size) {
    len = file_table[fd].size + file_table[fd].disk_offset - file_table[fd].fs_offset;
  }
  ramdisk_write(buf, file_table[fd].fs_offset, len);
  file_table[fd].fs_offset += len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  size_t new_offset;
  switch (whence)
  {
  case SEEK_SET:
    new_offset = file_table[fd].disk_offset + offset;
  break;
  case SEEK_CUR:
    new_offset = file_table[fd].fs_offset + offset;
  break;
  case SEEK_END:
    new_offset = file_table[fd].disk_offset + file_table[fd].size + offset;
    break;
  default:
    return -1;
  }
  if (new_offset >= file_table[fd].disk_offset && new_offset <= file_table[fd].disk_offset + file_table[fd].size){
    file_table[fd].fs_offset = new_offset;
    return new_offset - file_table[fd].disk_offset;
  } else {
    return -1;
  }
}

int fs_close(int fd) {
  return 0;
}