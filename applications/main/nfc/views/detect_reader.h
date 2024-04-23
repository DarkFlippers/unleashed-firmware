#pragma once
#include <stdint.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

typedef struct DetectReader DetectReader;

typedef enum {
    DetectReaderStateStart,
    DetectReaderStateReaderDetected,
    DetectReaderStateReaderLost,
    DetectReaderStateDone,
} DetectReaderState;

typedef void (*DetectReaderDoneCallback)(void* context);

DetectReader* detect_reader_alloc(void);

void detect_reader_free(DetectReader* detect_reader);

void detect_reader_reset(DetectReader* detect_reader);

View* detect_reader_get_view(DetectReader* detect_reader);

void detect_reader_set_callback(
    DetectReader* detect_reader,
    DetectReaderDoneCallback callback,
    void* context);

void detect_reader_set_nonces_max(DetectReader* detect_reader, uint16_t nonces_max);

void detect_reader_set_nonces_collected(DetectReader* detect_reader, uint16_t nonces_collected);

void detect_reader_set_state(DetectReader* detect_reader, DetectReaderState state);

void detect_reader_set_uid(DetectReader* detect_reader, uint8_t* uid, uint8_t uid_len);
