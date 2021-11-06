#pragma once

#include <stdint.h>

void clock_init();

uint32_t timer_micros();
uint32_t timer_millis();

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);