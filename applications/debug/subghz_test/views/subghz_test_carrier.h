#pragma once

#include <gui/view.h>
// #include <lib/subghz/devices/devices.h>

typedef enum {
    SubGhzTestCarrierEventOnlyRx,
} SubGhzTestCarrierEvent;

typedef struct SubGhzTestCarrier SubGhzTestCarrier;

typedef void (*SubGhzTestCarrierCallback)(SubGhzTestCarrierEvent event, void* context);

void subghz_test_carrier_set_callback(
    SubGhzTestCarrier* subghz_test_carrier,
    SubGhzTestCarrierCallback callback,
    void* context);

SubGhzTestCarrier* subghz_test_carrier_alloc();

void subghz_test_carrier_free(SubGhzTestCarrier* subghz_test_carrier);

View* subghz_test_carrier_get_view(SubGhzTestCarrier* subghz_test_carrier);

// void subghz_test_carrier_set_radio(
//     SubGhzTestCarrier* subghz_test_carrier,
//     const SubGhzDevice* radio_device);
