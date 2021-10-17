#pragma once

#include <furi-hal-i2c.h>
#include <furi-hal-light.h>
#include <furi-hal-resources.h>
#include <furi-hal-spi.h>

#define furi_assert(value) (void)(value)

void furi_hal_init();

void delay(float milliseconds);

void delay_us(float microseconds);
