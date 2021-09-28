#pragma once

#include <gui/view.h>

typedef enum {
    SubghzTestStaticEventOnlyRx,
} SubghzTestStaticEvent;

typedef struct SubghzTestStatic SubghzTestStatic;

typedef void (*SubghzTestStaticCallback)(SubghzTestStaticEvent event, void* context);

void subghz_test_static_set_callback(
    SubghzTestStatic* subghz_test_static,
    SubghzTestStaticCallback callback,
    void* context);

SubghzTestStatic* subghz_test_static_alloc();

void subghz_test_static_free(SubghzTestStatic* subghz_static);

View* subghz_test_static_get_view(SubghzTestStatic* subghz_static);
