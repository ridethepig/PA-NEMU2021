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
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

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
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

static size_t fs_offset;

int fs_open(const char* pathname, int flags, int mode) {
  int i;
  for (i = 0; i < sizeof(file_table) / sizeof(Finfo); ++ i) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      fs_offset = file_table[i].disk_offset;
      return i; // simply assign file_table index to fd
    }
  }
  panic("file not found");
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  if (file_table[fd].read) {
    len = file_table[fd].read(buf, fs_offset, len);
    fs_offset += len;
    return len;
  }
  // fallback to ramdisk read
  if (fs_offset - file_table[fd].disk_offset + len > file_table[fd].size) {
    len = file_table[fd].size + file_table[fd].disk_offset - fs_offset;
  }
  ramdisk_read(buf, fs_offset, len);
  fs_offset += len;
  return len;
}

size_t fs_write(int fd, void *buf, size_t len) {
  if (file_table[fd].write) {
    len = file_table[fd].write(buf, fs_offset,len);
    fs_offset += len;
    return len;
  }
  if (fs_offset - file_table[fd].disk_offset + len > file_table[fd].size) {
    len = file_table[fd].size + file_table[fd].disk_offset - fs_offset;
  }
  ramdisk_write(buf, fs_offset, len);
  fs_offset += len;
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
    new_offset = fs_offset + offset;
  break;
  case SEEK_END:
    new_offset = file_table[fd].disk_offset + file_table[fd].size + offset;
    break;
  default:
    return -1;
  }
  if (new_offset >= file_table[fd].disk_offset && new_offset <= file_table[fd].disk_offset + file_table[fd].size){
    fs_offset = new_offset;
    return new_offset - file_table[fd].disk_offset;
  } else {
    return -1;
  }
}

int fs_close(int fd) {
  return 0;
}