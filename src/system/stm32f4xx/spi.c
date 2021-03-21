#include "system/spi.h"

#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_spi.h>

static void spi_dma_rcc_enable(const spi_port_def_t *port) {
  switch (port->dma.dma_port) {
  case 1:
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    break;
  case 2:
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
    break;
  }
}

static void spi_rcc_enable(const spi_port_def_t *port) {
  switch (port->index) {
#if defined(SPI1)
  case 1:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    break;
#endif
#if defined(SPI2)
  case 2:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    break;
#endif
#if defined(SPI3)
  case 3:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
    break;
#endif
#if defined(SPI4)
  case 4:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI4);
    break;
#endif
#if defined(SPI5)
  case 5:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI5);
    break;
#endif
#if defined(SPI6)
  case 6:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI6);
    break;
#endif
  default:
    break;
  }
}

void spi_init(const spi_device_def_t *dev, uint32_t mode) {
  gpio_config(dev->port->sck.pin, GPIO_AF | GPIO_PP | GPIO_PULL_UP);
  gpio_config(dev->port->mosi.pin, GPIO_AF | GPIO_PP | GPIO_PULL_NONE);
  gpio_config(dev->port->miso.pin, GPIO_AF | GPIO_PP | GPIO_PULL_NONE);

  gpio_config_af(dev->port->sck.pin, dev->port->sck.af);
  gpio_config_af(dev->port->mosi.pin, dev->port->mosi.af);
  gpio_config_af(dev->port->miso.pin, dev->port->miso.af);

  gpio_config(dev->nss, GPIO_OUTPUT | GPIO_PP | GPIO_PULL_UP);
  gpio_set(dev->nss);

  spi_rcc_enable(dev->port);

  LL_SPI_Disable(dev->port->channel);
  LL_SPI_DeInit(dev->port->channel);

  LL_SPI_InitTypeDef init;
  LL_SPI_StructInit(&init);

  init.TransferDirection = LL_SPI_FULL_DUPLEX;
  init.Mode = LL_SPI_MODE_MASTER;
  init.DataWidth = LL_SPI_DATAWIDTH_8BIT;

  if (mode & SPI_CPOL_LOW) {
    init.ClockPolarity = LL_SPI_POLARITY_LOW;
  } else if (mode & SPI_CPOL_HIGH) {
    init.ClockPolarity = LL_SPI_POLARITY_HIGH;
  }

  if (mode & SPI_CPHA_1EDGE) {
    init.ClockPhase = LL_SPI_PHASE_1EDGE;
  } else if (mode & SPI_CPHA_2EDGE) {
    init.ClockPhase = LL_SPI_PHASE_2EDGE;
  }

  init.NSS = LL_SPI_NSS_SOFT;
  init.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
  init.BitOrder = LL_SPI_MSB_FIRST;

  LL_SPI_Init(dev->port->channel, &init);
  LL_SPI_Enable(dev->port->channel);
}

static void spi_init_dma(const spi_device_def_t *dev) {
  spi_dma_rcc_enable(dev->port);
  NVIC_EnableIRQ(dev->port->dma.rx_it);
}

void spi_csn_enable(const spi_device_def_t *dev) {
  gpio_reset(dev->nss);
}

void spi_csn_disable(const spi_device_def_t *dev) {
  gpio_set(dev->nss);
}

uint8_t spi_transfer_byte(const spi_device_def_t *dev, uint8_t data) {
  return spi_transfer_byte_timeout(dev, data, 0x400);
}

uint8_t spi_transfer_byte_timeout(const spi_device_def_t *dev, uint8_t data, uint32_t timeout_max) {
  for (uint16_t timeout = timeout_max; LL_SPI_IsActiveFlag_TXE(dev->port->channel) == RESET; timeout--) {
    if (timeout == 0) {
      return 0;
    }
  }

  LL_SPI_TransmitData8(dev->port->channel, data);

  for (uint16_t timeout = timeout_max; LL_SPI_IsActiveFlag_RXNE(dev->port->channel) == RESET; timeout--) {
    if (timeout == 0) {
      return 0;
    }
  }

  for (uint16_t timeout = timeout_max; LL_SPI_IsActiveFlag_BSY(dev->port->channel) == SET; timeout--) {
    if (timeout == 0) {
      return 0;
    }
  }

  return LL_SPI_ReceiveData8(dev->port->channel);
}