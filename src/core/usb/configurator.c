#include "core/usb/configurator.h"

#include "driver/reset.h"

void usb_update() {
  uint8_t data = 0;
  if (!usb_serial_get(&data)) {
    return;
  }

  if (data == USB_MAGIC_REBOOT) {
    system_reset_to_bootloader();
    return;
  }

  if (data == USB_MAGIC_SOFT_REBOOT) {
    system_reset();
    return;
  }
}