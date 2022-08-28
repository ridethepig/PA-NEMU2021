#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t cnt = 0;
  while (*s ++ != '\0') {
      cnt ++;
  }
  return cnt;
}

size_t strnlen(const char *s, size_t n) {
    size_t cnt = 0;
    while (cnt < n && *s ++ != '\0') {
        cnt ++;
    }
    return cnt;
}

char *strcpy(char *dst, const char *src) {
  char *p = dst;
  while ((*p ++ = *src ++) != '\0')
      /* nothing */;
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *p = dst;
  while (n > 0) {
      if ((*p = *src) != '\0') {
          src ++;
      }
      p ++, n --;
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char* _dst = dst;
  while (*dst != '\0') {
    dst ++;
  }
  while (*src != '\0') {
    *dst++ = *src++;
  }
  *dst = '\0'; // dont forget the \0
  return _dst; // return the uncorrupted ptr
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && *s1 == *s2) {
      s1 ++, s2 ++;
  }
  return (int)((unsigned char)*s1 - (unsigned char)*s2);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n > 0 && *s1 != '\0' && *s1 == *s2) {
      n --, s1 ++, s2 ++;
  }
  return (n == 0) ? 0 : (int)((unsigned char)*s1 - (unsigned char)*s2);
}

void *memset(void *s, int c, size_t n) {
  char *p = s;
  while (n -- > 0) {
      *p ++ = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  const char *s = src;
  char *d = dst;
  if (s < d && s + n > d) {
      s += n, d += n;
      while (n -- > 0) {
          *-- d = *-- s;
      }
  } else {
      while (n -- > 0) {
          *d ++ = *s ++;
      }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  const char *s = in;
  char *d = out;
  while (n -- > 0) {
      *d ++ = *s ++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *_s1 = (const char *)s1;
  const char *_s2 = (const char *)s2;
  while (n -- > 0) {
      if (*_s1 != *_s2) {
          return (int)((unsigned char)*_s1 - (unsigned char)*_s2);
      }
      _s1 ++, _s2 ++;
  }
  return 0;
}

#endif
