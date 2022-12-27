#pragma once

#include <gui/view.h>

typedef struct GpioReader GpioReader;
typedef void (*GpioReaderOkCallback)(InputType type, void* context);

GpioReader* gpio_reader_alloc();

void gpio_reader_free(GpioReader* gpio_reader);

View* gpio_reader_get_view(GpioReader* gpio_reader);

void gpio_reader_set_ok_callback(
    GpioReader* gpio_reader,
    GpioReaderOkCallback callback,
    void* context);
