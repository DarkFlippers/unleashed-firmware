#include "ring.h"
#include <flipper_v2.h>

struct Ring {
    uint8_t* data;
    size_t size;
    volatile size_t read_ptr;
    volatile size_t write_ptr;
};

Ring* ring_alloc(size_t size) {
    Ring* ring = furi_alloc(sizeof(Ring));
    ring->size = size + 1;
    ring->data = furi_alloc(ring->size);
    ring_clear(ring);
    return ring;
}

void ring_free(Ring* ring) {
    furi_assert(ring);
    free(ring->data);
    free(ring);
}

size_t ring_size(Ring* ring) {
    furi_assert(ring);
    return ring->size - 1;
}

inline static size_t ring_read_calculate(Ring* ring, size_t r, size_t w) {
    if(w >= r) {
        return w - r;
    } else {
        return ring->size - (r - w);
    }
}

size_t ring_read_space(Ring* ring) {
    furi_assert(ring);

    const size_t r = ring->read_ptr;
    const size_t w = ring->write_ptr;

    return ring_read_calculate(ring, r, w);
}

inline static size_t ring_write_calculate(Ring* ring, size_t r, size_t w) {
    if(r > w) {
        return r - w - 1;
    } else {
        return ring->size - (r - w);
    }
}

size_t ring_write_space(Ring* ring) {
    furi_assert(ring);

    const size_t r = ring->read_ptr;
    const size_t w = ring->write_ptr;

    return ring_write_calculate(ring, r, w);
}

size_t ring_push(Ring* ring, const uint8_t* data, size_t size) {
    furi_assert(ring);
    furi_assert(data);

    const size_t r = ring->read_ptr;
    size_t w = ring->write_ptr;
    const size_t write_space = ring_write_calculate(ring, r, w);

    if(write_space == 0) return 0;

    const size_t to_write = size > write_space ? write_space : size;
    size_t end, first, second;

    end = w + to_write;
    if(end > ring->size) {
        first = ring->size - w;
        second = end % ring->size;
    } else {
        first = to_write;
        second = 0;
    }

    memcpy(ring->data + w, data, first);
    w = (w + first) % ring->size;

    if(second) {
        memcpy(ring->data + w, data + first, second);
        w = (w + second) % ring->size;
    }

    ring->write_ptr = w;

    return to_write;
}

size_t ring_pull(Ring* ring, uint8_t* data, size_t size) {
    furi_assert(ring);
    furi_assert(data);

    size_t r = ring->read_ptr;
    const size_t w = ring->write_ptr;
    const size_t read_space = ring_read_calculate(ring, r, w);

    if(read_space == 0) return 0;

    size_t to_read = size > read_space ? read_space : size;
    size_t end, first, second;

    end = r + to_read;
    if(end > ring->size) {
        first = ring->size - r;
        second = end % ring->size;
    } else {
        first = to_read;
        second = 0;
    }

    memcpy(data, ring->data + r, first);
    r = (r + first) % ring->size;

    if(second) {
        memcpy(data + first, ring->data + r, second);
        r = (r + second) % ring->size;
    }

    ring->read_ptr = r;

    return to_read;
}

void ring_clear(Ring* ring) {
    furi_assert(ring);
    ring->read_ptr = 0;
    ring->write_ptr = 0;
}
