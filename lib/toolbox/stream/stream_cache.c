#include "stream_cache.h"

#define STREAM_CACHE_MAX_SIZE 1024U

struct StreamCache {
    uint8_t data[STREAM_CACHE_MAX_SIZE];
    size_t data_size;
    size_t position;
};

StreamCache* stream_cache_alloc() {
    StreamCache* cache = malloc(sizeof(StreamCache));
    cache->data_size = 0;
    cache->position = 0;
    return cache;
}
void stream_cache_free(StreamCache* cache) {
    furi_assert(cache);
    cache->data_size = 0;
    cache->position = 0;
    free(cache);
}

void stream_cache_drop(StreamCache* cache) {
    cache->data_size = 0;
    cache->position = 0;
}

bool stream_cache_at_end(StreamCache* cache) {
    furi_assert(cache->data_size >= cache->position);
    return cache->data_size == cache->position;
}

size_t stream_cache_size(StreamCache* cache) {
    return cache->data_size;
}

size_t stream_cache_pos(StreamCache* cache) {
    return cache->position;
}

size_t stream_cache_fill(StreamCache* cache, Stream* stream) {
    const size_t size_read = stream_read(stream, cache->data, STREAM_CACHE_MAX_SIZE);
    cache->data_size = size_read;
    cache->position = 0;
    return size_read;
}

size_t stream_cache_read(StreamCache* cache, uint8_t* data, size_t size) {
    furi_assert(cache->data_size >= cache->position);
    const size_t size_read = MIN(size, cache->data_size - cache->position);
    if(size_read > 0) {
        memcpy(data, cache->data + cache->position, size_read);
        cache->position += size_read;
    }
    return size_read;
}

int32_t stream_cache_seek(StreamCache* cache, int32_t offset) {
    furi_assert(cache->data_size >= cache->position);
    int32_t actual_offset = 0;

    if(offset > 0) {
        actual_offset = MIN(cache->data_size - cache->position, (size_t)offset);
    } else if(offset < 0) {
        actual_offset = -MIN(cache->position, (size_t)abs(offset));
    }

    cache->position += actual_offset;
    return actual_offset;
}
