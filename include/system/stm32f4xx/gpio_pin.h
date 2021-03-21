#pragma once

#include <stdint.h>

#include <stm32f4xx_ll_gpio.h>

struct gpio_pin_def {
  GPIO_TypeDef *port;
  uint8_t index;
  uint32_t bit_index;
};

struct gpio_af_pin_def {
  const struct gpio_pin_def *pin;
  uint32_t af;
};

#define MAKE_PIN_DEF(port, pin) static const struct gpio_pin_def P##port##pin = {GPIO##port, pin, (1U << pin)};
#include "gpio_pin.in"
#undef MAKE_PIN_DEF