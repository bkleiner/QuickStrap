#pragma once

#include <stdint.h>

typedef struct {
  uint8_t *const buffer;
  uint32_t head;
  uint32_t tail;
  const uint32_t size;
} ring_buffer_t;

uint32_t ring_buffer_free(ring_buffer_t *c);
uint8_t ring_buffer_put(ring_buffer_t *c, uint8_t data);
uint32_t ring_buffer_write(ring_buffer_t *c, const uint8_t *data, const uint32_t len);

uint32_t ring_buffer_available(ring_buffer_t *c);
uint8_t ring_buffer_get(ring_buffer_t *c, uint8_t *data);
uint32_t ring_buffer_read(ring_buffer_t *c, uint8_t *data, const uint32_t len);

void ring_buffer_clear(ring_buffer_t *c);