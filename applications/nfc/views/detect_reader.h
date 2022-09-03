#pragma once
#include <stdint.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

typedef struct DetectReader DetectReader;

typedef void (*DetectReaderDoneCallback)(void* context);

DetectReader* detect_reader_alloc();

void detect_reader_free(DetectReader* detect_reader);

void detect_reader_reset(DetectReader* detect_reader);

View* detect_reader_get_view(DetectReader* detect_reader);

void detect_reader_set_callback(
    DetectReader* detect_reader,
    DetectReaderDoneCallback callback,
    void* context);

void detect_reader_inc_nonce_cnt(DetectReader* detect_reader);
