#include "util/ring_buffer.h"

#include "math.h"

uint32_t ring_buffer_free(ring_buffer_t *c) {
  if (c->head >= c->tail) {
    return (c->size - c->head) + c->tail;
  }
  return (c->tail - c->head);
}

uint8_t ring_buffer_put(ring_buffer_t *c, uint8_t data) {
  uint32_t next = c->head + 1;
  if (next >= c->size)
    next = 0;

  if (next == c->tail)
    return 0;

  c->buffer[c->head] = data;
  c->head = next;
  return 1;
}

uint32_t ring_buffer_write(ring_buffer_t *c, const uint8_t *data, const uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    uint32_t next = (c->head + 1) % c->size;

    if (next == c->tail)
      return i;

    c->buffer[c->head] = data[i];
    c->head = next;
  }
  return len;
}

uint32_t ring_buffer_available(ring_buffer_t *c) {
  if (c->head >= c->tail) {
    return c->head - c->tail;
  }
  return c->size + c->head - c->tail;
}

uint8_t ring_buffer_get(ring_buffer_t *c, uint8_t *data) {
  if (c->head == c->tail)
    return 0;

  uint32_t next = c->tail + 1;
  if (next >= c->size)
    next = 0;

  *data = c->buffer[c->tail];
  c->tail = next;
  return 1;
}

uint32_t ring_buffer_read(ring_buffer_t *c, uint8_t *data, const uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    if (c->head == c->tail)
      return i;

    uint32_t next = (c->tail + 1) % c->size;
    data[i] = c->buffer[c->tail];
    c->tail = next;
  }
  return len;
}

void ring_buffer_clear(ring_buffer_t *c) {
  c->tail = c->head;
}
