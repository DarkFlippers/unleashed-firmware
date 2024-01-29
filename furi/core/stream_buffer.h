/**
 * @file stream_buffer.h
 * Furi stream buffer primitive.
 * 
 * Stream buffers are used to send a continuous stream of data from one task or
 * interrupt to another.  Their implementation is light weight, making them
 * particularly suited for interrupt to task and core to core communication
 * scenarios.
 * 
 * ***NOTE***: Stream buffer implementation assumes there is only one task or
 * interrupt that will write to the buffer (the writer), and only one task or
 * interrupt that will read from the buffer (the reader).
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void FuriStreamBuffer;

/**
 * @brief Allocate stream buffer instance.
 * Stream buffer implementation assumes there is only one task or
 * interrupt that will write to the buffer (the writer), and only one task or
 * interrupt that will read from the buffer (the reader).
 * 
 * @param size The total number of bytes the stream buffer will be able to hold at any one time.
 * @param trigger_level The number of bytes that must be in the stream buffer 
 * before a task that is blocked on the stream buffer to wait for data is moved out of the blocked state.
 * @return The stream buffer instance.
 */
FuriStreamBuffer* furi_stream_buffer_alloc(size_t size, size_t trigger_level);

/**
 * @brief Free stream buffer instance
 * 
 * @param stream_buffer The stream buffer instance.
 */
void furi_stream_buffer_free(FuriStreamBuffer* stream_buffer);

/**
 * @brief Set trigger level for stream buffer.
 * A stream buffer's trigger level is the number of bytes that must be in the
 * stream buffer before a task that is blocked on the stream buffer to
 * wait for data is moved out of the blocked state.
 * 
 * @param stream_buffer The stream buffer instance
 * @param trigger_level The new trigger level for the stream buffer.
 * @return true if trigger level can be be updated (new trigger level was less than or equal to the stream buffer's length). 
 * @return false if trigger level can't be be updated (new trigger level was greater than the stream buffer's length).
 */
bool furi_stream_set_trigger_level(FuriStreamBuffer* stream_buffer, size_t trigger_level);

/**
 * @brief Sends bytes to a stream buffer. The bytes are copied into the stream buffer.
 * Wakes up task waiting for data to become available if called from ISR.
 * 
 * @param stream_buffer The stream buffer instance.
 * @param data A pointer to the data that is to be copied into the stream buffer.
 * @param length The maximum number of bytes to copy from data into the stream buffer.
 * @param timeout The maximum amount of time the task should remain in the
 * Blocked state to wait for space to become available if the stream buffer is full. 
 * Will return immediately if timeout is zero. 
 * Setting timeout to FuriWaitForever will cause the task to wait indefinitely.
 * Ignored if called from ISR.
 * @return The number of bytes actually written to the stream buffer. 
 */
size_t furi_stream_buffer_send(
    FuriStreamBuffer* stream_buffer,
    const void* data,
    size_t length,
    uint32_t timeout);

/**
 * @brief Receives bytes from a stream buffer.
 * Wakes up task waiting for space to become available if called from ISR.
 * 
 * @param stream_buffer The stream buffer instance.
 * @param data A pointer to the buffer into which the received bytes will be
 * copied.
 * @param length The length of the buffer pointed to by the data parameter.
 * @param timeout The maximum amount of time the task should remain in the
 * Blocked state to wait for data to become available if the stream buffer is empty. 
 * Will return immediately if timeout is zero. 
 * Setting timeout to FuriWaitForever will cause the task to wait indefinitely.
 * Ignored if called from ISR.
 * @return The number of bytes read from the stream buffer, if any. 
 */
size_t furi_stream_buffer_receive(
    FuriStreamBuffer* stream_buffer,
    void* data,
    size_t length,
    uint32_t timeout);

/**
 * @brief Queries a stream buffer to see how much data it contains, which is equal to
 * the number of bytes that can be read from the stream buffer before the stream
 * buffer would be empty.
 * 
 * @param stream_buffer The stream buffer instance.
 * @return The number of bytes that can be read from the stream buffer before
 * the stream buffer would be empty.
 */
size_t furi_stream_buffer_bytes_available(FuriStreamBuffer* stream_buffer);

/**
 * @brief Queries a stream buffer to see how much free space it contains, which is
 * equal to the amount of data that can be sent to the stream buffer before it
 * is full.
 * 
 * @param stream_buffer The stream buffer instance.
 * @return The number of bytes that can be written to the stream buffer before
 * the stream buffer would be full. 
 */
size_t furi_stream_buffer_spaces_available(FuriStreamBuffer* stream_buffer);

/**
 * @brief Queries a stream buffer to see if it is full.
 * 
 * @param stream_buffer stream buffer instance.
 * @return true if the stream buffer is full.
 * @return false if the stream buffer is not full.
 */
bool furi_stream_buffer_is_full(FuriStreamBuffer* stream_buffer);

/**
 * @brief Queries a stream buffer to see if it is empty.
 * 
 * @param stream_buffer The stream buffer instance.
 * @return true if the stream buffer is empty.
 * @return false if the stream buffer is not empty.
 */
bool furi_stream_buffer_is_empty(FuriStreamBuffer* stream_buffer);

/**
 * @brief Resets a stream buffer to its initial, empty, state. Any data that was 
 * in the stream buffer is discarded. A stream buffer can only be reset if there 
 * are no tasks blocked waiting to either send to or receive from the stream buffer.
 * 
 * @param stream_buffer The stream buffer instance.
 * @return FuriStatusOk if the stream buffer is reset. 
 * @return FuriStatusError if there was a task blocked waiting to send to or read 
 * from the stream buffer then the stream buffer is not reset.
 */
FuriStatus furi_stream_buffer_reset(FuriStreamBuffer* stream_buffer);

#ifdef __cplusplus
}
#endif
