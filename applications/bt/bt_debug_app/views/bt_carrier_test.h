#pragma once
#include <gui/view.h>

typedef struct BtCarrierTest BtCarrierTest;

BtCarrierTest* bt_carrier_test_alloc();

void bt_carrier_test_free(BtCarrierTest* bt_carrier_test);

View* bt_carrier_test_get_view(BtCarrierTest* bt_carrier_test);
