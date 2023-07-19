#pragma once

#include <gui/view.h>
#include "../helpers/camera_suite_custom_event.h"

typedef struct CameraSuiteViewGuide CameraSuiteViewGuide;

typedef void (*CameraSuiteViewGuideCallback)(CameraSuiteCustomEvent event, void* context);

void camera_suite_view_guide_set_callback(
    CameraSuiteViewGuide* camera_suite_view_guide,
    CameraSuiteViewGuideCallback callback,
    void* context);

View* camera_suite_view_guide_get_view(CameraSuiteViewGuide* camera_suite_static);

CameraSuiteViewGuide* camera_suite_view_guide_alloc();

void camera_suite_view_guide_free(CameraSuiteViewGuide* camera_suite_static);