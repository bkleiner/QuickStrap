#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"

typedef enum {
  LED_1,
  LED_2,
} led_index_t;

typedef struct {
  uint8_t index;
  const gpio_pin_def_t *pin;
  bool invert;
} led_pin_def_t;

void led_init();

void led_on(led_index_t num);
void led_off(led_index_t num);