#include "driver/debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "driver/usb.h"

void debug_print(const char *str) {
  usb_serial_write((const uint8_t *)str, strlen(str));
}

void debug_printf(const char *fmt, ...) {
  const uint32_t size = strlen(fmt) + 256;
  char str[size];

  memset(str, 0, size);

  va_list args;
  va_start(args, fmt);
  vsnprintf(str, size, fmt, args);
  va_end(args);

  debug_print(str);
}