#include "system/clock.h"

#include <stm32.h>

#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_system.h>
#include <stm32f4xx_ll_utils.h>

void clock_init() {
  /* set flash latency 2WS */
  _BMD(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2WS);

  /* setting up PLL 16MHz HSI, VCO=144MHz, PLLP = 72MHz PLLQ = 48MHz  */
  _BMD(RCC->PLLCFGR,
       RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLP,
       _VAL2FLD(RCC_PLLCFGR_PLLM, 8) | _VAL2FLD(RCC_PLLCFGR_PLLN, 72) | _VAL2FLD(RCC_PLLCFGR_PLLQ, 3));

  /* enabling PLL */
  _BST(RCC->CR, RCC_CR_PLLON);
  _WBS(RCC->CR, RCC_CR_PLLRDY);

  /* switching to PLL */
  _BMD(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
  _WVL(RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL);

  SystemCoreClockUpdate();
  LL_Init1msTick(SystemCoreClock);

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t timer_micros() {
  static uint32_t total_micros = 0;
  static uint32_t last_micros = 0;

  const uint32_t micros = DWT->CYCCNT / (SystemCoreClock / 1000000L);
  if (micros >= last_micros) {
    total_micros += micros - last_micros;
  } else {
    total_micros += ((UINT32_MAX / (SystemCoreClock / 1000000L)) + micros) - last_micros;
  }

  last_micros = micros;
  return total_micros;
}

uint32_t timer_millis() {
  static uint32_t total_millis = 0;
  static uint32_t last_millis = 0;

  const uint32_t millis = timer_micros() / 1000;
  if (millis >= last_millis) {
    total_millis += millis - last_millis;
  } else {
    total_millis += ((UINT32_MAX / 1000) + millis) - last_millis;
  }

  last_millis = millis;
  return total_millis;
}

void delay_us(uint32_t us) {
  volatile uint32_t delay = us * (SystemCoreClock / 1000000L);
  volatile uint32_t start = DWT->CYCCNT;
  while (DWT->CYCCNT - start < delay) {
    __asm("NOP");
  }
}

void delay_ms(uint32_t ms) {
  delay_us(ms * 1000);
}