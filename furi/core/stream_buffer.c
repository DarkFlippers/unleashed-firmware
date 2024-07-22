#include "stream_buffer.h"

#include "check.h"
#include "common_defines.h"

#include <FreeRTOS.h>
#include <FreeRTOS-Kernel/include/stream_buffer.h>

struct FuriStreamBuffer {
    StaticStreamBuffer_t container;
    uint8_t buffer[];
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriStreamBuffer, container) == 0);
// IMPORTANT: buffer MUST be the LAST struct member
static_assert(offsetof(FuriStreamBuffer, buffer) == sizeof(FuriStreamBuffer));

FuriStreamBuffer* furi_stream_buffer_alloc(size_t size, size_t trigger_level) {
    furi_check(size != 0);

    // Actual FreeRTOS usable buffer size seems to be one less
    const size_t buffer_size = size + 1;

    FuriStreamBuffer* stream_buffer = malloc(sizeof(FuriStreamBuffer) + buffer_size);
    StreamBufferHandle_t hStreamBuffer = xStreamBufferCreateStatic(
        buffer_size, trigger_level, stream_buffer->buffer, &stream_buffer->container);

    furi_check(hStreamBuffer == (StreamBufferHandle_t)stream_buffer);

    return stream_buffer;
}

void furi_stream_buffer_free(FuriStreamBuffer* stream_buffer) {
    furi_check(stream_buffer);

    vStreamBufferDelete((StreamBufferHandle_t)stream_buffer);
    free(stream_buffer);
}

bool furi_stream_set_trigger_level(FuriStreamBuffer* stream_buffer, size_t trigger_level) {
    furi_check(stream_buffer);
    return xStreamBufferSetTriggerLevel((StreamBufferHandle_t)stream_buffer, trigger_level) ==
           pdTRUE;
}

size_t furi_stream_buffer_send(
    FuriStreamBuffer* stream_buffer,
    const void* data,
    size_t length,
    uint32_t timeout) {
    furi_check(stream_buffer);

    size_t ret;

    if(FURI_IS_IRQ_MODE()) {
        BaseType_t yield;
        ret = xStreamBufferSendFromISR((StreamBufferHandle_t)stream_buffer, data, length, &yield);
        portYIELD_FROM_ISR(yield);
    } else {
        ret = xStreamBufferSend((StreamBufferHandle_t)stream_buffer, data, length, timeout);
    }

    return ret;
}

size_t furi_stream_buffer_receive(
    FuriStreamBuffer* stream_buffer,
    void* data,
    size_t length,
    uint32_t timeout) {
    furi_check(stream_buffer);

    size_t ret;

    if(FURI_IS_IRQ_MODE()) {
        BaseType_t yield;
        ret =
            xStreamBufferReceiveFromISR((StreamBufferHandle_t)stream_buffer, data, length, &yield);
        portYIELD_FROM_ISR(yield);
    } else {
        ret = xStreamBufferReceive((StreamBufferHandle_t)stream_buffer, data, length, timeout);
    }

    return ret;
}

size_t furi_stream_buffer_bytes_available(FuriStreamBuffer* stream_buffer) {
    furi_check(stream_buffer);

    return xStreamBufferBytesAvailable((StreamBufferHandle_t)stream_buffer);
}

size_t furi_stream_buffer_spaces_available(FuriStreamBuffer* stream_buffer) {
    furi_check(stream_buffer);

    return xStreamBufferSpacesAvailable((StreamBufferHandle_t)stream_buffer);
}

bool furi_stream_buffer_is_full(FuriStreamBuffer* stream_buffer) {
    furi_check(stream_buffer);

    return xStreamBufferIsFull((StreamBufferHandle_t)stream_buffer) == pdTRUE;
}

bool furi_stream_buffer_is_empty(FuriStreamBuffer* stream_buffer) {
    furi_check(stream_buffer);

    return xStreamBufferIsEmpty((StreamBufferHandle_t)stream_buffer) == pdTRUE;
}

FuriStatus furi_stream_buffer_reset(FuriStreamBuffer* stream_buffer) {
    furi_check(stream_buffer);

    if(xStreamBufferReset((StreamBufferHandle_t)stream_buffer) == pdPASS) {
        return FuriStatusOk;
    } else {
        return FuriStatusError;
    }
}
