// ###########################
// GENERATED FILE, DO NOT EDIT
// TARGET: pedro_f4
// ###########################

#pragma once

#include "driver/stm32f4xx/system.h"
#include "driver/led.h"
#include "driver/gyro/gyro.h"

#define TARGET_NAME "pedro_f4"


#define USE_UART_PORT_1
#define USE_UART_PORT_2
#define USE_UART_PORT_3
#define USE_UART_PORT_4

enum uart_port_names {
  UART_PORT_1,
  UART_PORT_2,
  UART_PORT_3,
  UART_PORT_4,
  UART_PORT_MAX
};

static const struct uart_port_def uart_ports[UART_PORT_MAX] = {
  {
    .index = 1,
    .channel = USART1,
    .irq = USART1_IRQn,
    .rx = {
      .pin = &PA10,
      .af = 7,
    },
    .tx = {
      .pin = &PA9,
      .af = 7,
    },
  },
  {
    .index = 2,
    .channel = USART2,
    .irq = USART2_IRQn,
    .rx = {
      .pin = &PA3,
      .af = 7,
    },
    .tx = {
      .pin = &PA2,
      .af = 7,
    },
  },
  {
    .index = 3,
    .channel = USART3,
    .irq = USART3_IRQn,
    .rx = {
      .pin = &PC11,
      .af = 7,
    },
    .tx = {
      .pin = &PC10,
      .af = 7,
    },
  },
  {
    .index = 4,
    .channel = UART4,
    .irq = UART4_IRQn,
    .rx = {
      .pin = &PA1,
      .af = 8,
    },
    .tx = {
      .pin = &PA0,
      .af = 8,
    },
  },
};

enum spi_port_names {
  SPI_PORT_1,
  SPI_PORT_2,
  SPI_PORT_3,
  SPI_PORT_MAX
};

static const struct spi_port_def spi_ports[SPI_PORT_MAX] = {
  {
    .index = 1,
    .channel = SPI1,
    .sck = {
      .pin = &PA5,
      .af = 5,
    },
    .miso = {
      .pin = &PA6,
      .af = 5,
    },
    .mosi = {
      .pin = &PA7,
      .af = 5,
    },
    .dma = {
      .dma_port = 2,
      .channel = LL_DMA_CHANNEL_4,
      .index = 4,
      .rx_stream_index = 2,
      .rx_stream = DMA2_Stream2,
      .rx_tci_flag = DMA_LISR_TCIF2,
      .rx_it = DMA2_Stream2_IRQn,
      .rx_it_flag = DMA_LIFCR_CTCIF2,
      .tx_stream_index = 3,
      .tx_stream = DMA2_Stream3,
      .tx_tci_flag = DMA_LISR_TCIF3,
      .tx_it = DMA2_Stream3_IRQn,
      .tx_it_flag = DMA_LIFCR_CTCIF3,
    },
  },
  {
    .index = 2,
    .channel = SPI2,
    .sck = {
      .pin = &PB13,
      .af = 5,
    },
    .miso = {
      .pin = &PB14,
      .af = 5,
    },
    .mosi = {
      .pin = &PB15,
      .af = 5,
    },
    .dma = {
      .dma_port = 1,
      .channel = LL_DMA_CHANNEL_0,
      .index = 0,
      .rx_stream_index = 3,
      .rx_stream = DMA1_Stream3,
      .rx_tci_flag = DMA_LISR_TCIF3,
      .rx_it = DMA1_Stream3_IRQn,
      .rx_it_flag = DMA_LIFCR_CTCIF3,
      .tx_stream_index = 4,
      .tx_stream = DMA1_Stream4,
      .tx_tci_flag = DMA_HISR_TCIF4,
      .tx_it = DMA1_Stream4_IRQn,
      .tx_it_flag = DMA_HIFCR_CTCIF4,
    },
  },
  {
    .index = 3,
    .channel = SPI3,
    .sck = {
      .pin = &PB3,
      .af = 6,
    },
    .miso = {
      .pin = &PB4,
      .af = 6,
    },
    .mosi = {
      .pin = &PB5,
      .af = 6,
    },
    .dma = {
      .dma_port = 1,
      .channel = LL_DMA_CHANNEL_0,
      .index = 0,
      .rx_stream_index = 0,
      .rx_stream = DMA1_Stream0,
      .rx_tci_flag = DMA_LISR_TCIF0,
      .rx_it = DMA1_Stream0_IRQn,
      .rx_it_flag = DMA_LIFCR_CTCIF0,
      .tx_stream_index = 7,
      .tx_stream = DMA1_Stream7,
      .tx_tci_flag = DMA_HISR_TCIF7,
      .tx_it = DMA1_Stream7_IRQn,
      .tx_it_flag = DMA_HIFCR_CTCIF7,
    },
  },
};

enum led_pin_names {
  LED_PIN_1,
  LED_PIN_MAX
};

static const led_pin_def_t led_defs[LED_PIN_MAX] = {
  {
    .index = 1,
    .pin = &PC12,
    .invert = 1,
  },
};

enum gyro_names {
  GYRO_1,
  GYRO_MAX
};

static const spi_device_def_t gyro_defs[GYRO_MAX] = {
  {
    .port = &spi_ports[0],
    .nss = &PA4,
  },
};