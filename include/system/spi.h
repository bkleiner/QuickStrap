#pragma once

#include <stdint.h>

#include "gpio.h"

#include "stm32f4xx/spi_port.h"

enum spi_mode {
  SPI_CPOL_LOW = (1 << 1),
  SPI_CPOL_HIGH = (1 << 2),

  SPI_CPHA_1EDGE = (1 << 3),
  SPI_CPHA_2EDGE = (1 << 4),
};

typedef struct spi_port_def spi_port_def_t;

typedef struct {
  const spi_port_def_t *port;
  const gpio_pin_def_t *nss;
} spi_device_def_t;

void spi_init(const spi_device_def_t *dev, uint32_t mode);

void spi_csn_enable(const spi_device_def_t *dev);
void spi_csn_disable(const spi_device_def_t *dev);

uint8_t spi_transfer_byte(const spi_device_def_t *dev, uint8_t data);
uint8_t spi_transfer_byte_timeout(const spi_device_def_t *dev, uint8_t data, uint32_t timeout_max);