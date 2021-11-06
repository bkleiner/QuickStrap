#include "driver/gyro/mpu6xxx.h"

#include "util/math.h"

#include "target/target.h"

#include "driver/clock.h"
#include "driver/spi.h"

#define MPU_BIT_SLEEP 0x40
#define MPU_BIT_H_RESET 0x80
#define MPU_BITS_CLKSEL 0x07
#define MPU_CLK_SEL_PLLGYROX 0x01
#define MPU_RESET_SIGNAL_PATHWAYS 0x07
#define MPU_EXT_SYNC_GYROX 0x02
#define MPU_BITS_STDBY_MODE_OFF 0x00
#define MPU_BITS_FS_250DPS 0x00
#define MPU_BITS_FS_500DPS 0x08
#define MPU_BITS_FS_1000DPS 0x10
#define MPU_BITS_FS_2000DPS 0x18
#define MPU_BITS_FS_2G 0x00
#define MPU_BITS_FS_4G 0x08
#define MPU_BITS_FS_8G 0x10
#define MPU_BITS_FS_16G 0x18
#define MPU_BITS_FS_MASK 0x18
#define MPU_BITS_DLPF_CFG_256HZ 0x00
#define MPU_BITS_DLPF_CFG_188HZ 0x01
#define MPU_BITS_DLPF_CFG_98HZ 0x02
#define MPU_BITS_DLPF_CFG_42HZ 0x03
#define MPU_BITS_DLPF_CFG_20HZ 0x04
#define MPU_BITS_DLPF_CFG_10HZ 0x05
#define MPU_BITS_DLPF_CFG_5HZ 0x06
#define MPU_BITS_DLPF_CFG_2100HZ_NOLPF 0x07
#define MPU_BITS_DLPF_CFG_MASK 0x07
#define MPU_BIT_INT_ANYRD_2CLEAR 0x10
#define MPU_BIT_RAW_RDY_EN 0x01
#define MPU_BIT_I2C_IF_DIS 0x10
#define MPU_BIT_INT_STATUS_DATA 0x01
#define MPU_BIT_SMPLRT_DIVIDER_OFF 0x00
#define MPU_BIT_GYRO 0x04
#define MPU_BIT_ACC 0x02
#define MPU_BIT_TEMP 0x01

#define MPU_RA_XG_OFFS_TC 0x00  //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU_RA_YG_OFFS_TC 0x01  //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU_RA_ZG_OFFS_TC 0x02  //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU_RA_X_FINE_GAIN 0x03 //[7:0] X_FINE_GAIN
#define MPU_RA_Y_FINE_GAIN 0x04 //[7:0] Y_FINE_GAIN
#define MPU_RA_Z_FINE_GAIN 0x05 //[7:0] Z_FINE_GAIN
#define MPU_RA_XA_OFFS_H 0x06   //[15:0] XA_OFFS
#define MPU_RA_XA_OFFS_L_TC 0x07
#define MPU_RA_YA_OFFS_H 0x08 //[15:0] YA_OFFS
#define MPU_RA_YA_OFFS_L_TC 0x09
#define MPU_RA_ZA_OFFS_H 0x0A //[15:0] ZA_OFFS
#define MPU_RA_ZA_OFFS_L_TC 0x0B
#define MPU_RA_PRODUCT_ID 0x0C   // Product ID Register
#define MPU_RA_XG_OFFS_USRH 0x13 //[15:0] XG_OFFS_USR
#define MPU_RA_XG_OFFS_USRL 0x14
#define MPU_RA_YG_OFFS_USRH 0x15 //[15:0] YG_OFFS_USR
#define MPU_RA_YG_OFFS_USRL 0x16
#define MPU_RA_ZG_OFFS_USRH 0x17 //[15:0] ZG_OFFS_USR
#define MPU_RA_ZG_OFFS_USRL 0x18
#define MPU_RA_SMPLRT_DIV 0x19
#define MPU_RA_CONFIG 0x1A
#define MPU_RA_GYRO_CONFIG 0x1B
#define MPU_RA_ACCEL_CONFIG 0x1C
#define MPU_RA_FF_THR 0x1D
#define MPU_RA_FF_DUR 0x1E
#define MPU_RA_MOT_THR 0x1F
#define MPU_RA_MOT_DUR 0x20
#define MPU_RA_ZRMOT_THR 0x21
#define MPU_RA_ZRMOT_DUR 0x22
#define MPU_RA_FIFO_EN 0x23
#define MPU_RA_I2C_MST_CTRL 0x24
#define MPU_RA_I2C_SLV0_ADDR 0x25
#define MPU_RA_I2C_SLV0_REG 0x26
#define MPU_RA_I2C_SLV0_CTRL 0x27
#define MPU_RA_I2C_SLV1_ADDR 0x28
#define MPU_RA_I2C_SLV1_REG 0x29
#define MPU_RA_I2C_SLV1_CTRL 0x2A
#define MPU_RA_I2C_SLV2_ADDR 0x2B
#define MPU_RA_I2C_SLV2_REG 0x2C
#define MPU_RA_I2C_SLV2_CTRL 0x2D
#define MPU_RA_I2C_SLV3_ADDR 0x2E
#define MPU_RA_I2C_SLV3_REG 0x2F
#define MPU_RA_I2C_SLV3_CTRL 0x30
#define MPU_RA_I2C_SLV4_ADDR 0x31
#define MPU_RA_I2C_SLV4_REG 0x32
#define MPU_RA_I2C_SLV4_DO 0x33
#define MPU_RA_I2C_SLV4_CTRL 0x34
#define MPU_RA_I2C_SLV4_DI 0x35
#define MPU_RA_I2C_MST_STATUS 0x36
#define MPU_RA_INT_PIN_CFG 0x37
#define MPU_RA_INT_ENABLE 0x38
#define MPU_RA_DMP_INT_STATUS 0x39
#define MPU_RA_INT_STATUS 0x3A
#define MPU_RA_ACCEL_XOUT_H 0x3B
#define MPU_RA_ACCEL_XOUT_L 0x3C
#define MPU_RA_ACCEL_YOUT_H 0x3D
#define MPU_RA_ACCEL_YOUT_L 0x3E
#define MPU_RA_ACCEL_ZOUT_H 0x3F
#define MPU_RA_ACCEL_ZOUT_L 0x40
#define MPU_RA_TEMP_OUT_H 0x41
#define MPU_RA_TEMP_OUT_L 0x42
#define MPU_RA_GYRO_XOUT_H 0x43
#define MPU_RA_GYRO_XOUT_L 0x44
#define MPU_RA_GYRO_YOUT_H 0x45
#define MPU_RA_GYRO_YOUT_L 0x46
#define MPU_RA_GYRO_ZOUT_H 0x47
#define MPU_RA_GYRO_ZOUT_L 0x48
#define MPU_RA_EXT_SENS_DATA_00 0x49
#define MPU_RA_MOT_DETECT_STATUS 0x61
#define MPU_RA_I2C_SLV0_DO 0x63
#define MPU_RA_I2C_SLV1_DO 0x64
#define MPU_RA_I2C_SLV2_DO 0x65
#define MPU_RA_I2C_SLV3_DO 0x66
#define MPU_RA_I2C_MST_DELAY_CTRL 0x67
#define MPU_RA_SIGNAL_PATH_RESET 0x68
#define MPU_RA_MOT_DETECT_CTRL 0x69
#define MPU_RA_USER_CTRL 0x6A
#define MPU_RA_PWR_MGMT_1 0x6B
#define MPU_RA_PWR_MGMT_2 0x6C
#define MPU_RA_BANK_SEL 0x6D
#define MPU_RA_MEM_START_ADDR 0x6E
#define MPU_RA_MEM_R_W 0x6F
#define MPU_RA_DMP_CFG_1 0x70
#define MPU_RA_DMP_CFG_2 0x71
#define MPU_RA_FIFO_COUNTH 0x72
#define MPU_RA_FIFO_COUNTL 0x73
#define MPU_RA_FIFO_R_W 0x74
#define MPU_RA_WHO_AM_I 0x75

// gyro has +-2000 divided over 16bit.
#define GYRO_RANGE 1.f / (65536.f / 4000.f)

#define ACCEL_RANGE (1.f / 2048.0f)

static uint8_t mpu6xxx_read(const spi_device_def_t *dev, uint8_t reg) {
  spi_csn_enable(dev);
  spi_transfer_byte(dev, reg | 0x80);
  uint8_t val = spi_transfer_byte(dev, 0x00);
  spi_csn_disable(dev);
  return val;
}

static void mpu6xxx_write(const spi_device_def_t *dev, uint8_t reg, uint8_t data) {
  spi_csn_enable(dev);
  spi_transfer_byte(dev, reg);
  spi_transfer_byte(dev, data);
  spi_csn_disable(dev);
}

static void mpu6xxx_read_multi(const spi_device_def_t *dev, uint8_t reg, uint8_t *data, uint32_t size) {
  spi_csn_enable(dev);

  spi_transfer_byte(dev, reg | 0x80);
  for (uint32_t i = 0; i < size; i++) {
    data[i] = spi_transfer_byte(dev, 0x00);
  }

  spi_csn_disable(dev);
}

static void mpu6xxx_init_device(const spi_device_def_t *dev) {
  spi_init(dev, SPI_CPOL_HIGH | SPI_CPHA_2EDGE);

  mpu6xxx_write(dev, MPU_RA_PWR_MGMT_1, MPU_BIT_H_RESET); //reg 107 soft reset  MPU_BIT_H_RESET
  delay_us(100000);
  mpu6xxx_write(dev, MPU_RA_SIGNAL_PATH_RESET, MPU_RESET_SIGNAL_PATHWAYS);
  delay_us(100000);
  mpu6xxx_write(dev, MPU_RA_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROX); //reg 107 set pll clock to 1 for x axis reference
  delay_us(100000);
  mpu6xxx_write(dev, MPU_RA_USER_CTRL, MPU_BIT_I2C_IF_DIS); //reg 106 to 16 enabling spi
  delay_us(1500);
  mpu6xxx_write(dev, MPU_RA_PWR_MGMT_2, MPU_BITS_STDBY_MODE_OFF); //reg 108 disable standbye mode to 0
  delay_us(1500);
  mpu6xxx_write(dev, MPU_RA_SMPLRT_DIV, MPU_BIT_SMPLRT_DIVIDER_OFF); //reg 25 sample rate divider to 0
  delay_us(1500);
  mpu6xxx_write(dev, MPU_RA_CONFIG, MPU_BITS_DLPF_CFG_256HZ); //reg 26 dlpf to 0 - 8khz
  delay_us(1500);
  mpu6xxx_write(dev, MPU_RA_ACCEL_CONFIG, MPU_BITS_FS_16G); //reg 28 accel scale to 16G
  delay_us(1500);
  mpu6xxx_write(dev, MPU_RA_GYRO_CONFIG, MPU_BITS_FS_2000DPS); //reg 27 gyro scale to 2000deg/s
  delay_us(1500);
  mpu6xxx_write(dev, MPU_RA_INT_ENABLE, MPU_BIT_INT_STATUS_DATA); //reg 56 data ready enable interrupt to 1
  delay_us(1500);
}

void mpu6xxx_init() {
  mpu6xxx_init_device(&gyro_defs[0]);
}

uint8_t mpu6xxx_read_id() {
  return mpu6xxx_read(&gyro_defs[0], MPU_RA_WHO_AM_I);
}

uint8_t mpu6xxx_read_data(gyro_data_t *data) {
  uint8_t raw[14];
  mpu6xxx_read_multi(&gyro_defs[0], MPU_RA_ACCEL_XOUT_H, raw, 14);

  data->accel[0] = (int16_t)((raw[0] << 8) | raw[1]) * ACCEL_RANGE;
  data->accel[1] = (int16_t)((raw[2] << 8) | raw[3]) * ACCEL_RANGE;
  data->accel[2] = (int16_t)((raw[4] << 8) | raw[5]) * ACCEL_RANGE;

  data->temp = (int16_t)((raw[6] << 8) | raw[7]) / 333.87f + 21.f;

  data->gyro[0] = (int16_t)((raw[8] << 8) | raw[9]) * GYRO_RANGE * DEGTORAD;
  data->gyro[1] = (int16_t)((raw[10] << 8) | raw[11]) * GYRO_RANGE * DEGTORAD;
  data->gyro[2] = (int16_t)((raw[12] << 8) | raw[13]) * GYRO_RANGE * DEGTORAD;

  return 1;
}