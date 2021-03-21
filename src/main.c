#include <stm32.h>

#include "target/target.h"

#include "system/clock.h"
#include "system/gpio.h"
#include "system/reset.h"

#include "system/uart.h"

#include "driver/debug.h"
#include "driver/gyro/gyro.h"
#include "driver/led.h"

#include "core/usb/configurator.h"

int main(void) {
  system_check_for_bootloader();

  clock_init();
  gpio_init();

  led_init();
  usb_init();

  uart_init(
      &uart_ports[UART_PORT_1],
      115200,
      UART_MODE_FULL_DUPLEX | UART_MODE_PARITY_NONE | UART_MODE_STOP_BITS_1);

  gyro_init();

  static gyro_data_t gyro;

  while (1) {
    gyro_read_data(&gyro);

    debug_printf(
        "GYRO: %d %d %d\n\r",
        (int16_t)(gyro.gyro[0] * 100.f),
        (int16_t)(gyro.gyro[1] * 100.f),
        (int16_t)(gyro.gyro[2] * 100.f));

    usb_update();
    delay_ms(100);
  }
}
