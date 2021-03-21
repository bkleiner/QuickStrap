#pragma once

#include "system/spi.h"

typedef struct {
  float gyro[3];
  float accel[3];
  float temp;
} gyro_data_t;

void gyro_init();
uint8_t gyro_read_data(gyro_data_t *data);