#include "driver/reset.h"

#include <stdint.h>

#include <stm32f4xx_ll_pwr.h>
#include <stm32f4xx_ll_rtc.h>
#include <stm32f4xx_ll_system.h>

void system_reset() {
  NVIC_SystemReset();
}

#define BOOTLOADER_MAGIC 0xDEADBEEF
#define BKP_INDEX LL_RTC_BKP_DR4

static void backup_register_write(uint32_t val) {
  __HAL_RCC_PWR_CLK_ENABLE();

  LL_PWR_EnableBkUpAccess();
  LL_RTC_BAK_SetRegister(RTC, BKP_INDEX, val);
  LL_PWR_DisableBkUpAccess();
}

void system_reset_to_bootloader() {
  backup_register_write(BOOTLOADER_MAGIC);
  system_reset();
}

void system_check_for_bootloader() {
  const uint32_t magic = LL_RTC_BAK_GetRegister(RTC, BKP_INDEX);
  if (magic != BOOTLOADER_MAGIC) {
    return;
  }

  backup_register_write(0);

  void (*DfuBootJump)(void) = (void (*)(void))(*((uint32_t *)0x1FFF0004));
  __set_MSP(*((uint32_t *)0x1FFF0000));
  DfuBootJump();

  while (1)
    ;
}