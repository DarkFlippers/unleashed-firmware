#pragma once

#include <gui/view.h>

typedef struct SubghzTestStatic SubghzTestStatic;

SubghzTestStatic* subghz_test_static_alloc();

void subghz_test_static_free(SubghzTestStatic* subghz_static);

View* subghz_test_static_get_view(SubghzTestStatic* subghz_static);
