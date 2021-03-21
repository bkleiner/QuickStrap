#pragma once

#include "system/usb.h"

typedef enum {
  USB_MAGIC_REBOOT = 'R',
  USB_MAGIC_SOFT_REBOOT = 'S',
} usb_magic_t;

void usb_update();