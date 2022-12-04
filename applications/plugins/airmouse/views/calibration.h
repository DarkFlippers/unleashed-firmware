#pragma once

#include <gui/view.h>
#include <gui/view_dispatcher.h>

typedef struct Calibration Calibration;

Calibration* calibration_alloc(ViewDispatcher* view_dispatcher);

void calibration_free(Calibration* calibration);

View* calibration_get_view(Calibration* calibration);
