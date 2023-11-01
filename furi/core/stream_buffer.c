#include "base.h"
#include "check.h"
#include "stream_buffer.h"
#include "common_defines.h"

#include <FreeRTOS.h>
#include <FreeRTOS-Kernel/include/stream_buffer.h>

FuriStreamBuffer* furi_stream_buffer_alloc(size_t size, size_t trigger_level) {
    furi_assert(size != 0);

    StreamBufferHandle_t handle = xStreamBufferCreate(size, trigger_level);
    furi_check(handle);

    return handle;
};

void furi_stream_buffer_free(FuriStreamBuffer* stream_buffer) {
    furi_assert(stream_buffer);
    vStreamBufferDelete(stream_buffer);
};

bool furi_stream_set_trigger_level(FuriStreamBuffer* stream_buffer, size_t trigger_level) {
    furi_assert(stream_buffer);
    return xStreamBufferSetTriggerLevel(stream_buffer, trigger_level) == pdTRUE;
};

size_t furi_stream_buffer_send(
    FuriStreamBuffer* stream_buffer,
    const void* data,
    size_t length,
    uint32_t timeout) {
    size_t ret;

    if(FURI_IS_IRQ_MODE()) {
        BaseType_t yield;
        ret = xStreamBufferSendFromISR(stream_buffer, data, length, &yield);
        portYIELD_FROM_ISR(yield);
    } else {
        ret = xStreamBufferSend(stream_buffer, data, length, timeout);
    }

    return ret;
};

size_t furi_stream_buffer_receive(
    FuriStreamBuffer* stream_buffer,
    void* data,
    size_t length,
    uint32_t timeout) {
    size_t ret;

    if(FURI_IS_IRQ_MODE()) {
        BaseType_t yield;
        ret = xStreamBufferReceiveFromISR(stream_buffer, data, length, &yield);
        portYIELD_FROM_ISR(yield);
    } else {
        ret = xStreamBufferReceive(stream_buffer, data, length, timeout);
    }

    return ret;
}

size_t furi_stream_buffer_bytes_available(FuriStreamBuffer* stream_buffer) {
    return xStreamBufferBytesAvailable(stream_buffer);
};

size_t furi_stream_buffer_spaces_available(FuriStreamBuffer* stream_buffer) {
    return xStreamBufferSpacesAvailable(stream_buffer);
};

bool furi_stream_buffer_is_full(FuriStreamBuffer* stream_buffer) {
    return xStreamBufferIsFull(stream_buffer) == pdTRUE;
};

bool furi_stream_buffer_is_empty(FuriStreamBuffer* stream_buffer) {
    return (xStreamBufferIsEmpty(stream_buffer) == pdTRUE);
};

FuriStatus furi_stream_buffer_reset(FuriStreamBuffer* stream_buffer) {
    if(xStreamBufferReset(stream_buffer) == pdPASS) {
        return FuriStatusOk;
    } else {
        return FuriStatusError;
    }
}