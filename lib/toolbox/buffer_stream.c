#include "buffer_stream.h"
#include <stream_buffer.h>

struct Buffer {
    volatile bool occupied;
    volatile size_t size;
    uint8_t* data;
    size_t max_data_size;
};

struct BufferStream {
    size_t stream_overrun_count;
    StreamBufferHandle_t stream;

    size_t index;
    Buffer* buffers;
    size_t max_buffers_count;
};

bool buffer_write(Buffer* buffer, const uint8_t* data, size_t size) {
    if(buffer->occupied) {
        return false;
    }
    if((buffer->size + size) > buffer->max_data_size) {
        return false;
    }
    memcpy(buffer->data + buffer->size, data, size);
    buffer->size += size;
    return true;
}

uint8_t* buffer_get_data(Buffer* buffer) {
    return buffer->data;
}

size_t buffer_get_size(Buffer* buffer) {
    return buffer->size;
}

void buffer_reset(Buffer* buffer) {
    buffer->occupied = false;
    buffer->size = 0;
}

BufferStream* buffer_stream_alloc(size_t buffer_size, size_t buffers_count) {
    furi_assert(buffer_size > 0);
    furi_assert(buffers_count > 0);
    BufferStream* buffer_stream = malloc(sizeof(BufferStream));
    buffer_stream->max_buffers_count = buffers_count;
    buffer_stream->buffers = malloc(sizeof(Buffer) * buffer_stream->max_buffers_count);
    for(size_t i = 0; i < buffer_stream->max_buffers_count; i++) {
        buffer_stream->buffers[i].occupied = false;
        buffer_stream->buffers[i].size = 0;
        buffer_stream->buffers[i].data = malloc(buffer_size);
        buffer_stream->buffers[i].max_data_size = buffer_size;
    }
    buffer_stream->stream = xStreamBufferCreate(
        sizeof(BufferStream*) * buffer_stream->max_buffers_count, sizeof(BufferStream*));
    buffer_stream->stream_overrun_count = 0;
    buffer_stream->index = 0;

    return buffer_stream;
}

void buffer_stream_free(BufferStream* buffer_stream) {
    for(size_t i = 0; i < buffer_stream->max_buffers_count; i++) {
        free(buffer_stream->buffers[i].data);
    }
    vStreamBufferDelete(buffer_stream->stream);
    free(buffer_stream->buffers);
    free(buffer_stream);
}

static inline int8_t buffer_stream_get_free_buffer(BufferStream* buffer_stream) {
    int8_t id = -1;
    for(size_t i = 0; i < buffer_stream->max_buffers_count; i++) {
        if(buffer_stream->buffers[i].occupied == false) {
            id = i;
            break;
        }
    }

    return id;
}

bool buffer_stream_send_from_isr(
    BufferStream* buffer_stream,
    const uint8_t* data,
    size_t size,
    BaseType_t* const task_woken) {
    Buffer* buffer = &buffer_stream->buffers[buffer_stream->index];
    bool result = true;

    // write to buffer
    if(!buffer_write(buffer, data, size)) {
        // if buffer is full - send it
        buffer->occupied = true;
        // we always have space for buffer in stream
        xStreamBufferSendFromISR(buffer_stream->stream, &buffer, sizeof(Buffer*), task_woken);

        // get new buffer from the pool
        int8_t index = buffer_stream_get_free_buffer(buffer_stream);

        // check that we have valid buffer
        if(index == -1) {
            // no free buffer
            buffer_stream->stream_overrun_count++;
            result = false;
        } else {
            // write to new buffer
            buffer_stream->index = index;
            buffer = &buffer_stream->buffers[buffer_stream->index];
            buffer_write(buffer, data, size);
        }
    }

    return result;
}

Buffer* buffer_stream_receive(BufferStream* buffer_stream, TickType_t timeout) {
    Buffer* buffer;
    size_t size = xStreamBufferReceive(buffer_stream->stream, &buffer, sizeof(Buffer*), timeout);

    if(size == sizeof(Buffer*)) {
        return buffer;
    } else {
        return NULL;
    }
}

size_t buffer_stream_get_overrun_count(BufferStream* buffer_stream) {
    return buffer_stream->stream_overrun_count;
}

void buffer_stream_reset(BufferStream* buffer_stream) {
    FURI_CRITICAL_ENTER();
    BaseType_t xReturn = xStreamBufferReset(buffer_stream->stream);
    furi_assert(xReturn == pdPASS);
    UNUSED(xReturn);
    buffer_stream->stream_overrun_count = 0;
    for(size_t i = 0; i < buffer_stream->max_buffers_count; i++) {
        buffer_reset(&buffer_stream->buffers[i]);
    }
    FURI_CRITICAL_EXIT();
}