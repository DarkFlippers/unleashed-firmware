#pragma once

#include <stdint.h>
#include <string.h>

typedef struct Ring Ring;

Ring* ring_alloc(size_t size);

void ring_free(Ring* ring);

size_t ring_size(Ring* ring);

size_t ring_read_space(Ring* ring);

size_t ring_write_space(Ring* ring);

size_t ring_push(Ring* ring, const uint8_t* data, size_t size);

size_t ring_pull(Ring* ring, uint8_t* data, size_t size);

void ring_clear(Ring* ring);
