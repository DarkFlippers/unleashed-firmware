#pragma once

#include <gui/view.h>

typedef struct SubghzTestCarrier SubghzTestCarrier;

SubghzTestCarrier* subghz_test_carrier_alloc();

void subghz_test_carrier_free(SubghzTestCarrier* subghz_test_carrier);

View* subghz_test_carrier_get_view(SubghzTestCarrier* subghz_test_carrier);
