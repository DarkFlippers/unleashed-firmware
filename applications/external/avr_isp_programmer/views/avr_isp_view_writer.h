#pragma once

#include <gui/view.h>
#include "../helpers/avr_isp_types.h"
#include "../helpers/avr_isp_event.h"

typedef struct AvrIspWriterView AvrIspWriterView;

typedef void (*AvrIspWriterViewCallback)(AvrIspCustomEvent event, void* context);

typedef enum {
    AvrIspWriterViewStatusIDLE,
    AvrIspWriterViewStatusWriting,
    AvrIspWriterViewStatusVerification,
    AvrIspWriterViewStatusWritingFuse,
    AvrIspWriterViewStatusWritingFuseOk,
} AvrIspWriterViewStatus;

void avr_isp_writer_update_progress(AvrIspWriterView* instance);

void avr_isp_writer_set_file_path(
    AvrIspWriterView* instance,
    const char* file_path,
    const char* file_name);

void avr_isp_writer_view_set_callback(
    AvrIspWriterView* instance,
    AvrIspWriterViewCallback callback,
    void* context);

AvrIspWriterView* avr_isp_writer_view_alloc();

void avr_isp_writer_view_free(AvrIspWriterView* instance);

View* avr_isp_writer_view_get_view(AvrIspWriterView* instance);

void avr_isp_writer_view_exit(void* context);
