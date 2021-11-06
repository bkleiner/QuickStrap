#pragma once

#include <stdint.h>

enum gpio_mode {
  GPIO_INPUT = (1 << 1),
  GPIO_OUTPUT = (1 << 2),
  GPIO_AF = (1 << 3),

  GPIO_PP = (1 << 4),
  GPIO_OD = (1 << 5),

  GPIO_PULL_DOWN = (1 << 6),
  GPIO_PULL_UP = (1 << 7),
  GPIO_PULL_NONE = (1 << 8),
};

typedef struct gpio_pin_def gpio_pin_def_t;

void gpio_init();
void gpio_config(const gpio_pin_def_t *pin, uint32_t mode);
void gpio_config_af(const gpio_pin_def_t *pin, uint32_t af);

void gpio_set(const gpio_pin_def_t *pin);
void gpio_reset(const gpio_pin_def_t *pin);