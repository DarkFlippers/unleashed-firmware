#pragma once

#include <gui/view.h>

typedef enum {
    SubGhzTestStaticEventOnlyRx,
} SubGhzTestStaticEvent;

typedef struct SubGhzTestStatic SubGhzTestStatic;

typedef void (*SubGhzTestStaticCallback)(SubGhzTestStaticEvent event, void* context);

void subghz_test_static_set_callback(
    SubGhzTestStatic* subghz_test_static,
    SubGhzTestStaticCallback callback,
    void* context);

SubGhzTestStatic* subghz_test_static_alloc(void);

void subghz_test_static_free(SubGhzTestStatic* subghz_static);

View* subghz_test_static_get_view(SubGhzTestStatic* subghz_static);
