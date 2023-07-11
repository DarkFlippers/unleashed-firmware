#pragma once

#include <gui/view.h>
#include "../helpers/camera_suite_custom_event.h"

typedef struct CameraSuiteViewStyle2 CameraSuiteViewStyle2;

typedef void (*CameraSuiteViewStyle2Callback)(CameraSuiteCustomEvent event, void* context);

void camera_suite_view_style_2_set_callback(
    CameraSuiteViewStyle2* instance,
    CameraSuiteViewStyle2Callback callback,
    void* context);

CameraSuiteViewStyle2* camera_suite_view_style_2_alloc();

void camera_suite_view_style_2_free(CameraSuiteViewStyle2* camera_suite_static);

View* camera_suite_view_style_2_get_view(CameraSuiteViewStyle2* boilerpate_static);
