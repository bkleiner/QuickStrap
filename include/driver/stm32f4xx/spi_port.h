#pragma once

#include <stm32f4xx_ll_dma.h>
#include <stm32f4xx_ll_spi.h>

#include "gpio_pin.h"

typedef struct {
  uint32_t dma_port;
  uint32_t channel;
  uint8_t index;

  uint8_t rx_stream_index;
  DMA_Stream_TypeDef *rx_stream;
  uint32_t rx_tci_flag;
  IRQn_Type rx_it;
  uint32_t rx_it_flag;

  uint8_t tx_stream_index;
  DMA_Stream_TypeDef *tx_stream;
  uint32_t tx_tci_flag;
  IRQn_Type tx_it;
  uint32_t tx_it_flag;
} spi_dma_def_t;

struct spi_port_def {
  uint8_t index;
  SPI_TypeDef *channel;

  const struct gpio_af_pin_def sck;
  const struct gpio_af_pin_def miso;
  const struct gpio_af_pin_def mosi;

  const spi_dma_def_t dma;
};