#pragma once

#include <stdint.h>

void usb_init();

uint8_t usb_serial_put(uint8_t data);
uint8_t usb_serial_get(uint8_t *data);

uint8_t usb_serial_write(const uint8_t *data, const uint32_t len);