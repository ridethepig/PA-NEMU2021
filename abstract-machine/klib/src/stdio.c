#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static unsigned long long getuint(va_list* ap, int lflag) {
  if (lflag >= 2) {
    return va_arg(*ap, unsigned long long);
  } else if (lflag) {
    return va_arg(*ap, unsigned long);
  } else {
    return va_arg(*ap, unsigned int);
  }
}
static long long getint(va_list* ap, int lflag) {
  if (lflag >= 2) {
    return va_arg(*ap, long long);
  } else if (lflag) {
    return va_arg(*ap, long);
  } else {
    return va_arg(*ap, int);
  }
}
static void printnum(void (*_putch)(char, void*), void* cnt, unsigned int num,
                     unsigned int base, int width, int padc) {
  unsigned int result = num;
  unsigned int mod = 0;
  if (base == 10) {
    unsigned int t = __divu10(result);
    mod = result - __mulu10(t);
    result = t;
  } else if (base == 8) {
    mod = result & 0x7;
    result = result >> 3;
  } else {
    mod = result & 0xF;
    result = result >> 4;
  }

  // first recursively print all preceding (more significant) digits
  if (num >= base) {
    printnum(_putch, cnt, result, base, width - 1, padc);
  } else {
    // print any needed pad characters before first digit
    while (--width > 0) {
      _putch(padc, cnt);
    }
  }
  // then print this (the least significant) digit
  _putch("0123456789abcdef"[mod], cnt);
}
static void vprintfmt(void (*_putch)(char, void*), void* cnt, const char* fmt,
                      va_list ap) {
  register const char* p;
  register int ch;
  unsigned long long num;
  int base, width, precision, lflag, altflag;
  while (1) {
    while ((ch = *(unsigned char*)fmt++) != '%') {
      if (ch == '\0') {
        return;
      }
      _putch(ch, cnt);
    }

    // Process a %-escape sequence
    char padc = ' ';
    width = precision = -1;
    lflag = altflag = 0;

  reswitch:
    switch (ch = *(unsigned char*)fmt++) {
      // flag to pad on the right
      case '-':
        padc = '-';
        goto reswitch;

      // flag to pad with 0's instead of spaces
      case '0':
        padc = '0';
        goto reswitch;

      // width field
      case '1' ... '9':
        for (precision = 0;; ++fmt) {
          precision = precision * 10 + ch - '0';
          ch = *fmt;
          if (ch < '0' || ch > '9') {
            break;
          }
        }
        goto process_precision;

      case '*':
        precision = va_arg(ap, int);
        goto process_precision;

      case '.':
        if (width < 0) width = 0;
        goto reswitch;

      case '#':
        altflag = 1;
        goto reswitch;

      process_precision:
        if (width < 0) width = precision, precision = -1;
        goto reswitch;

      // long flag (doubled for long long)
      case 'l':
        lflag++;
        goto reswitch;

      // character
      case 'c':
        _putch(va_arg(ap, int), cnt);
        break;
      // string
      case 's':
        if ((p = va_arg(ap, char*)) == NULL) {
          p = "(null)";
        }
        if (width > 0 && padc != '-') {
          for (width -= strnlen(p, precision); width > 0; width--) {
            _putch(padc, cnt);
          }
        }
        for (; (ch = *p++) != '\0' && (precision < 0 || --precision >= 0);
             width--) {
          if (altflag && (ch < ' ' || ch > '~')) {
            _putch('?', cnt);
          } else {
            _putch(ch, cnt);
          }
        }
        for (; width > 0; width--) {
          _putch(' ', cnt);
        }
        break;

      // (signed) decimal
      case 'd':
        num = getint(&ap, lflag);
        if ((long long)num < 0) {
          _putch('-', cnt);
          num = -(long long)num;
        }
        base = 10;
        goto number;
      // pointer
      case 'p':
        _putch('0', cnt);
        _putch('x', cnt);
        num = (unsigned long long)(uintptr_t)va_arg(ap, void*);
        base = 16;
        goto number;

      // (unsigned) hexadecimal
      case 'x':
        num = getuint(&ap, lflag);
        base = 16;
      number:
        printnum(_putch, cnt, num, base, width, padc);
        break;

      // escaped '%' character
      case '%':
        _putch(ch, cnt);
        break;

      // unrecognized escape sequence - just print it literally
      default:
        _putch('%', cnt);
        for (fmt--; fmt[-1] != '%'; fmt--) /* do nothing */;
        break;
    }
  }
}

static void putch_cnt(int ch, int* cnt) {
  putch(ch);
  (*cnt)++;
}

static int vprintf(const char* fmt, va_list ap) {
  int cnt = 0;
  vprintfmt((void*)putch_cnt, &cnt, fmt, ap);
  return cnt;
}

int printf(const char* fmt, ...) {
  va_list ap;
  int cnt;
  va_start(ap, fmt);
  cnt = vprintf(fmt, ap);
  va_end(ap);
  return cnt;
}

typedef struct {
  char* buf_s;
  char* buf_e;
  int cnt;
} sprintbuf;

static void putch_str(int ch, sprintbuf* buf) {
  buf->cnt++;
  *buf->buf_s++ = ch;
}

int vsprintf(char* out, const char* fmt, va_list ap) {
  sprintbuf buf;
  buf.buf_s = out;
  buf.buf_e = NULL;  // we are not using this without size specified
  buf.cnt = 0;
  vprintfmt((void *)putch_str, &buf, fmt, ap);
  *(buf.buf_s) = '\0';
  return buf.cnt;
}

int sprintf(char* out, const char* fmt, ...) {
  va_list ap;
  int cnt;
  va_start(ap, fmt);
  cnt = vsprintf(out, fmt, ap);
  va_end(ap);
  return cnt;
}

static void putch_strn(int ch, sprintbuf* buf) {
  if (buf->buf_s < buf->buf_e) {
    (buf->cnt)++;
    *buf->buf_s++ = ch;
  }
}

int vsnprintf(char* out, size_t n, const char* fmt, va_list ap) {
  sprintbuf buf;
  buf.buf_s = out;
  buf.buf_e = out + n - 1;
  buf.cnt = 0;
  vprintfmt((void*)putch_strn, &buf, fmt, ap);
  *(buf.buf_s) = '\0';
  return buf.cnt;
}

int snprintf(char* out, size_t n, const char* fmt, ...) {
  va_list ap;
  int cnt;
  va_start(ap, fmt);
  cnt = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return cnt;
}

#endif
