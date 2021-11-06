#include "driver/led.h"

#include "driver/gpio.h"
#include "target/target.h"

void led_init() {
  for (uint8_t i = 0; i < LED_PIN_MAX; i++) {
    gpio_config(led_defs[i].pin, GPIO_OUTPUT | GPIO_PP);
    led_off(i);
  }
}

void led_on(led_index_t i) {
  if (led_defs[i].invert) {
    gpio_reset(led_defs[i].pin);
  } else {
    gpio_set(led_defs[i].pin);
  }
}

void led_off(led_index_t i) {
  if (led_defs[i].invert) {
    gpio_set(led_defs[i].pin);
  } else {
    gpio_reset(led_defs[i].pin);
  }
}