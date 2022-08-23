/**
 * @file buffer_stream.h
 * 
 * This file implements the concept of a buffer stream.
 * Data is written to the buffer until the buffer is full.
 * Then the buffer pointer is written to the stream, and the new write buffer is taken from the buffer pool.
 * After the buffer has been read by the receiving thread, it is sent to the free buffer pool.
 * 
 * This will speed up sending large chunks of data between threads, compared to using a stream directly.
 */
#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Buffer Buffer;

/**
 * @brief Get buffer data pointer
 * @param buffer 
 * @return uint8_t* 
 */
uint8_t* buffer_get_data(Buffer* buffer);

/**
 * @brief Get buffer size
 * @param buffer 
 * @return size_t 
 */
size_t buffer_get_size(Buffer* buffer);

/**
 * @brief Reset buffer and send to free buffer pool
 * @param buffer 
 */
void buffer_reset(Buffer* buffer);

typedef struct BufferStream BufferStream;

/**
 * @brief Allocate a new BufferStream instance
 * @param buffer_size 
 * @param buffers_count 
 * @return BufferStream* 
 */
BufferStream* buffer_stream_alloc(size_t buffer_size, size_t buffers_count);

/**
 * @brief Free a BufferStream instance
 * @param buffer_stream 
 */
void buffer_stream_free(BufferStream* buffer_stream);

/**
 * @brief Write data to buffer stream, from ISR context
 * Data will be written to the buffer until the buffer is full, and only then will the buffer be sent.
 * @param buffer_stream 
 * @param data 
 * @param size 
 * @param task_woken 
 * @return bool 
 */
bool buffer_stream_send_from_isr(
    BufferStream* buffer_stream,
    const uint8_t* data,
    size_t size,
    BaseType_t* const task_woken);

/**
 * @brief Receive buffer from stream
 * @param buffer_stream 
 * @param timeout 
 * @return Buffer* 
 */
Buffer* buffer_stream_receive(BufferStream* buffer_stream, TickType_t timeout);

/**
 * @brief Get stream overrun count
 * @param buffer_stream 
 * @return size_t 
 */
size_t buffer_stream_get_overrun_count(BufferStream* buffer_stream);

/**
 * @brief Reset stream and buffer pool
 * @param buffer_stream 
 */
void buffer_stream_reset(BufferStream* buffer_stream);

#ifdef __cplusplus
}
#endif
