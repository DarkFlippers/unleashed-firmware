#pragma once

#include <gui/view.h>
#include "../helpers/avr_isp_types.h"
#include "../helpers/avr_isp_event.h"

typedef struct AvrIspChipDetectView AvrIspChipDetectView;

typedef void (*AvrIspChipDetectViewCallback)(AvrIspCustomEvent event, void* context);

typedef enum {
    AvrIspChipDetectViewStateNoDetect,
    AvrIspChipDetectViewStateDetecting,
    AvrIspChipDetectViewStateDetected,
    AvrIspChipDetectViewStateErrorOccured,
    AvrIspChipDetectViewStateErrorVerification,
} AvrIspChipDetectViewState;

void avr_isp_chip_detect_view_set_callback(
    AvrIspChipDetectView* instance,
    AvrIspChipDetectViewCallback callback,
    void* context);

void avr_isp_chip_detect_set_state(AvrIspChipDetectView* instance, AvrIspChipDetectViewState state);

AvrIspChipDetectView* avr_isp_chip_detect_view_alloc();

void avr_isp_chip_detect_view_free(AvrIspChipDetectView* instance);

View* avr_isp_chip_detect_view_get_view(AvrIspChipDetectView* instance);

void avr_isp_chip_detect_view_exit(void* context);
