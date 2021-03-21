#pragma once

#include <stdint.h>

#include "driver/gyro/gyro.h"

void mpu6xxx_init();

uint8_t mpu6xxx_read_id();
uint8_t mpu6xxx_read_data(gyro_data_t *data);