#include "driver/gyro/gyro.h"

#include "driver/gyro/mpu6xxx.h"

void gyro_init() {
  mpu6xxx_init();
}

uint8_t gyro_read_data(gyro_data_t *data) {
  return mpu6xxx_read_data(data);
}