#pragma once

#include <stdint.h>

#include "gpio.h"

#include "stm32f4xx/uart_port.h"

enum uart_mode {
  UART_MODE_HALF_DUPLEX = (1 << 1),
  UART_MODE_FULL_DUPLEX = (1 << 2),

  UART_MODE_STOP_BITS_1 = (1 << 3),
  UART_MODE_STOP_BITS_2 = (1 << 4),

  UART_MODE_PARITY_NONE = (1 << 5),
  UART_MODE_PARITY_EVEN = (1 << 6),
  UART_MODE_PARITY_ODD = (1 << 7),
};

typedef struct uart_port_def uart_port_def_t;

void uart_init(const uart_port_def_t *dev, uint32_t baudrate, uint32_t mode);
