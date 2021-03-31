#pragma once

#include <gui/view.h>

typedef struct SubghzTestBasic SubghzTestBasic;

SubghzTestBasic* subghz_test_basic_alloc();

void subghz_test_basic_free(SubghzTestBasic* subghz_test_basic);

View* subghz_test_basic_get_view(SubghzTestBasic* subghz_test_basic);
