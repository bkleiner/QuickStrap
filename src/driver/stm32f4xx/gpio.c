#include "driver/gpio.h"

#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_rcc.h>

#include "driver/stm32f4xx/gpio_pin.h"

void gpio_init() {
  SET_BIT(
      RCC->AHB1ENR,
      RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN |
          RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN |
          RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN | RCC_AHB1ENR_GPIOIEN);
}

void gpio_config(const gpio_pin_def_t *pin, uint32_t mode) {
  if (mode & GPIO_INPUT) {
    LL_GPIO_SetPinMode(pin->port, pin->bit_index, LL_GPIO_MODE_INPUT);
  } else if (mode & GPIO_OUTPUT) {
    LL_GPIO_SetPinMode(pin->port, pin->bit_index, LL_GPIO_MODE_OUTPUT);
  } else if (mode & GPIO_AF) {
    LL_GPIO_SetPinMode(pin->port, pin->bit_index, LL_GPIO_MODE_ALTERNATE);
  }

  if (mode & GPIO_PP) {
    LL_GPIO_SetPinOutputType(pin->port, pin->bit_index, LL_GPIO_OUTPUT_PUSHPULL);
  } else if (mode & GPIO_OD) {
    LL_GPIO_SetPinOutputType(pin->port, pin->bit_index, LL_GPIO_OUTPUT_OPENDRAIN);
  }

  if (mode & GPIO_PULL_DOWN) {
    LL_GPIO_SetPinPull(pin->port, pin->bit_index, LL_GPIO_PULL_UP);
  } else if (mode & GPIO_PULL_UP) {
    LL_GPIO_SetPinPull(pin->port, pin->bit_index, LL_GPIO_PULL_DOWN);
  } else if (mode & GPIO_PULL_NONE) {
    LL_GPIO_SetPinPull(pin->port, pin->bit_index, LL_GPIO_PULL_NO);
  }

  LL_GPIO_SetPinSpeed(pin->port, pin->bit_index, GPIO_SPEED_HIGH);
}

void gpio_config_af(const gpio_pin_def_t *pin, uint32_t af) {
  if (pin->index > 7) {
    LL_GPIO_SetAFPin_8_15(pin->port, pin->bit_index, af);
  } else {
    LL_GPIO_SetAFPin_0_7(pin->port, pin->bit_index, af);
  }
}

void gpio_set(const gpio_pin_def_t *pin) {
  WRITE_REG(pin->port->BSRR, (GPIO_BSRR_BS_0 << pin->index));
}

void gpio_reset(const gpio_pin_def_t *pin) {
  WRITE_REG(pin->port->BSRR, (GPIO_BSRR_BR_0 << pin->index));
}