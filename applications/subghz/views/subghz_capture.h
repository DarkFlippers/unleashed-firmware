#pragma once

#include <gui/view.h>

typedef struct SubghzCapture SubghzCapture;

SubghzCapture* subghz_capture_alloc();

void subghz_capture_free(SubghzCapture* subghz_capture);

View* subghz_capture_get_view(SubghzCapture* subghz_capture);
