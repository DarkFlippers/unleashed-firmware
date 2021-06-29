#pragma once

#include <gui/view.h>

typedef struct SubghzStatic SubghzStatic;

SubghzStatic* subghz_static_alloc();

void subghz_static_free(SubghzStatic* subghz_static);

View* subghz_static_get_view(SubghzStatic* subghz_static);
