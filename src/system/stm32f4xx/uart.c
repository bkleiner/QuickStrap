#include "system/uart.h"

#include "target/target.h"

#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_usart.h>

static void uart_rcc_enable(const uart_port_def_t *port) {
  switch (port->index) {
#if defined(USART1)
  case 1:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    break;
#endif
#if defined(USART2)
  case 2:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    break;
#endif
#if defined(USART3)
  case 3:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
    break;
#endif
#if defined(UART4)
  case 4:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
    break;
#endif
#if defined(UART5)
  case 5:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
    break;
#endif
#if defined(USART6)
  case 6:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
    break;
#endif
  default:
    break;
  }
}

void uart_init(const uart_port_def_t *dev, uint32_t baudrate, uint32_t mode) {
  gpio_config(dev->rx.pin, GPIO_AF | GPIO_PP | GPIO_PULL_UP);
  gpio_config(dev->tx.pin, GPIO_AF | GPIO_PP | GPIO_PULL_UP);

  gpio_config_af(dev->rx.pin, dev->rx.af);
  gpio_config_af(dev->tx.pin, dev->tx.af);

  uart_rcc_enable(dev);

  LL_USART_Disable(dev->channel);
  LL_USART_DeInit(dev->channel);

  LL_USART_InitTypeDef init;
  LL_USART_StructInit(&init);

  init.BaudRate = baudrate;
  init.DataWidth = LL_USART_DATAWIDTH_8B;

  if (mode & UART_MODE_STOP_BITS_1) {
    init.StopBits = LL_USART_STOPBITS_1;
  } else if (mode & UART_MODE_STOP_BITS_2) {
    init.StopBits = LL_USART_STOPBITS_2;
  }

  if (mode & UART_MODE_PARITY_NONE) {
    init.Parity = LL_USART_PARITY_NONE;
  } else if (mode & UART_MODE_PARITY_EVEN) {
    init.Parity = LL_USART_PARITY_EVEN;
  } else if (mode & UART_MODE_PARITY_ODD) {
    init.Parity = LL_USART_PARITY_ODD;
  }

  init.TransferDirection = LL_USART_DIRECTION_TX_RX;
  init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  init.OverSampling = LL_USART_OVERSAMPLING_16;

  if (mode & UART_MODE_HALF_DUPLEX) {
    LL_USART_ConfigHalfDuplexMode(dev->channel);
  }

  NVIC_EnableIRQ(dev->irq);

  LL_USART_Init(dev->channel, &init);
  LL_USART_Enable(dev->channel);
  LL_USART_EnableIT_RXNE(dev->channel);
}

static void uart_isr_handler(const uart_port_def_t *dev) {
  if (LL_USART_IsActiveFlag_RXNE(dev->channel)) {
    volatile uint8_t val = LL_USART_ReceiveData8(dev->channel);
    LL_USART_TransmitData8(dev->channel, val);
  }
}

#if defined(USART1) && defined(USE_UART_PORT_1)
void USART1_IRQHandler() {
  uart_isr_handler(&uart_ports[UART_PORT_1]);
}
#endif

#if defined(USART2) && defined(USE_UART_PORT_2)
void USART2_IRQHandler() {
  uart_isr_handler(&uart_ports[UART_PORT_2]);
}
#endif

#if defined(USART3) && defined(USE_UART_PORT_3)
void USART3_IRQHandler() {
  uart_isr_handler(&uart_ports[UART_PORT_3]);
}
#endif

#if defined(UART4) && defined(USE_UART_PORT_4)
void UART4_IRQHandler() {
  uart_isr_handler(&uart_ports[UART_PORT_4]);
}
#endif

#if defined(UART5) && defined(USE_UART_PORT_5)
void UART5_IRQHandler() {
  uart_isr_handler(&uart_ports[UART_PORT_5]);
}
#endif

#if defined(USART6) && defined(USE_UART_PORT_6)
void USART6_IRQHandler() {
  uart_isr_handler(&uart_ports[UART_PORT_6]);
}
#endif
