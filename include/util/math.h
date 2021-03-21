#pragma once

#include <stdint.h>

#define DEGTORAD 0.017453292f
#define RADTODEG 57.29577951f

static inline uint32_t min_u32(uint32_t a, uint32_t b) {
  return a < b ? a : b;
}
static inline uint32_t max_u32(uint32_t a, uint32_t b) {
  return a > b ? a : b;
}

static inline int32_t min_i32(int32_t a, int32_t b) {
  return a < b ? a : b;
}
static inline int32_t max_i32(int32_t a, int32_t b) {
  return a > b ? a : b;
}

static inline uint16_t min_u16(uint16_t a, uint16_t b) {
  return a < b ? a : b;
}
static inline uint16_t max_u16(uint16_t a, uint16_t b) {
  return a > b ? a : b;
}

static inline int16_t min_i16(int16_t a, int16_t b) {
  return a < b ? a : b;
}
static inline int16_t max_i16(int16_t a, int16_t b) {
  return a > b ? a : b;
}

static inline uint8_t min_u8(uint8_t a, uint8_t b) {
  return a < b ? a : b;
}
static inline uint8_t max_u8(uint8_t a, uint8_t b) {
  return a > b ? a : b;
}

static inline int8_t min_i8(int8_t a, int8_t b) {
  return a < b ? a : b;
}
static inline int8_t max_i8(int8_t a, int8_t b) {
  return a > b ? a : b;
}