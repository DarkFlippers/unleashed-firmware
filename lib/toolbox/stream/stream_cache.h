#pragma once

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct StreamCache StreamCache;

/**
 * Allocate stream cache.
 * @return StreamCache* pointer to a StreamCache instance
 */
StreamCache* stream_cache_alloc();

/**
 * Free stream cache.
 * @param cache Pointer to a StreamCache instance
 */
void stream_cache_free(StreamCache* cache);

/**
 * Drop the cache contents and set it to initial state.
 * @param cache Pointer to a StreamCache instance
 */
void stream_cache_drop(StreamCache* cache);

/**
 * Determine if the internal cursor is at end the end of cached data.
 * @param cache Pointer to a StreamCache instance
 * @return True if cursor is at end, otherwise false.
 */
bool stream_cache_at_end(StreamCache* cache);

/**
 * Get the current size of cached data.
 * @param cache Pointer to a StreamCache instance
 * @return Size of cached data.
 */
size_t stream_cache_size(StreamCache* cache);

/**
 * Get the internal cursor position.
 * @param cache Pointer to a StreamCache instance
 * @return Cursor position inside the cache.
 */
size_t stream_cache_pos(StreamCache* cache);

/**
 * Load the cache with new data from a stream.
 * @param cache Pointer to a StreamCache instance
 * @param stream Pointer to a Stream instance
 * @return Size of newly cached data.
 */
size_t stream_cache_fill(StreamCache* cache, Stream* stream);

/**
 * Write as much cached data as possible to a stream.
 * @param cache Pointer to a StreamCache instance
 * @param stream Pointer to a Stream instance
 * @return True on success, False on failure.
 */
bool stream_cache_flush(StreamCache* cache, Stream* stream);

/**
 * Read cached data and advance the internal cursor.
 * @param cache Pointer to a StreamCache instance.
 * @param data Pointer to a data buffer. Must be initialized.
 * @param size Maximum size in bytes to read from the cache.
 * @return Actual size that was read.
 */
size_t stream_cache_read(StreamCache* cache, uint8_t* data, size_t size);

/**
 * Write to cached data and advance the internal cursor.
 * @param cache Pointer to a StreamCache instance.
 * @param data Pointer to a data buffer.
 * @param size Maximum size in bytes to write to the cache.
 * @return Actual size that was written.
 */
size_t stream_cache_write(StreamCache* cache, const uint8_t* data, size_t size);

/**
 * Move the internal cursor relatively to its current position.
 * @param cache Pointer to a StreamCache instance.
 * @param offset Cursor offset.
 * @return Actual cursor offset. Equal to offset parameter on hit.
 */
int32_t stream_cache_seek(StreamCache* cache, int32_t offset);

#ifdef __cplusplus
}
#endif
