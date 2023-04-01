#pragma once

#include <gui/view.h>
#include "../helpers/avr_isp_types.h"
#include "../helpers/avr_isp_event.h"

typedef struct AvrIspReaderView AvrIspReaderView;

typedef void (*AvrIspReaderViewCallback)(AvrIspCustomEvent event, void* context);

typedef enum {
    AvrIspReaderViewStatusIDLE,
    AvrIspReaderViewStatusReading,
    AvrIspReaderViewStatusVerification,
} AvrIspReaderViewStatus;

void avr_isp_reader_update_progress(AvrIspReaderView* instance);

void avr_isp_reader_set_file_path(
    AvrIspReaderView* instance,
    const char* file_path,
    const char* file_name);

void avr_isp_reader_view_set_callback(
    AvrIspReaderView* instance,
    AvrIspReaderViewCallback callback,
    void* context);

AvrIspReaderView* avr_isp_reader_view_alloc();

void avr_isp_reader_view_free(AvrIspReaderView* instance);

View* avr_isp_reader_view_get_view(AvrIspReaderView* instance);

void avr_isp_reader_view_exit(void* context);
