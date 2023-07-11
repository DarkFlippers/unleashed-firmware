#include "../helpers/camera_suite_custom_event.h"
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_console.h>
#include <furi_hal_uart.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/modules/dialog_ex.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <storage/filesystem_api_defines.h>
#include <storage/storage.h>

#pragma once

#define FRAME_WIDTH 128
#define FRAME_HEIGHT 64
#define FRAME_BIT_DEPTH 1
#define FRAME_BUFFER_LENGTH \
    (FRAME_WIDTH * FRAME_HEIGHT * FRAME_BIT_DEPTH / 8) // 128*64*1 / 8 = 1024
#define ROW_BUFFER_LENGTH (FRAME_WIDTH / 8) // 128/8 = 16
#define RING_BUFFER_LENGTH (ROW_BUFFER_LENGTH + 3) // ROW_BUFFER_LENGTH + Header => 16 + 3 = 19
#define LAST_ROW_INDEX (FRAME_BUFFER_LENGTH - ROW_BUFFER_LENGTH) // 1024 - 16 = 1008

typedef struct UartDumpModel UartDumpModel;

struct UartDumpModel {
    bool initialized;
    int rotation_angle;
    uint8_t pixels[FRAME_BUFFER_LENGTH];
    uint8_t ringbuffer_index;
    uint8_t row_ringbuffer[RING_BUFFER_LENGTH];
};

typedef struct CameraSuiteViewStyle1 CameraSuiteViewStyle1;

typedef void (*CameraSuiteViewStyle1Callback)(CameraSuiteCustomEvent event, void* context);

void camera_suite_view_style_1_set_callback(
    CameraSuiteViewStyle1* camera_suite_view_style_1,
    CameraSuiteViewStyle1Callback callback,
    void* context);

CameraSuiteViewStyle1* camera_suite_view_style_1_alloc();

void camera_suite_view_style_1_free(CameraSuiteViewStyle1* camera_suite_static);

View* camera_suite_view_style_1_get_view(CameraSuiteViewStyle1* camera_suite_static);

typedef enum {
    // Reserved for StreamBuffer internal event
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
    WorkerEventRx = (1 << 2),
} WorkerEventFlags;

#define WORKER_EVENTS_MASK (WorkerEventStop | WorkerEventRx)
