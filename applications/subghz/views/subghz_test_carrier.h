#pragma once

#include <gui/view.h>

typedef enum {
    SubghzTestCarrierEventOnlyRx,
} SubghzTestCarrierEvent;

typedef struct SubghzTestCarrier SubghzTestCarrier;

typedef void (*SubghzTestCarrierCallback)(SubghzTestCarrierEvent event, void* context);

void subghz_test_carrier_set_callback(
    SubghzTestCarrier* subghz_test_carrier,
    SubghzTestCarrierCallback callback,
    void* context);

SubghzTestCarrier* subghz_test_carrier_alloc();

void subghz_test_carrier_free(SubghzTestCarrier* subghz_test_carrier);

View* subghz_test_carrier_get_view(SubghzTestCarrier* subghz_test_carrier);
