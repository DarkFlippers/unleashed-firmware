#pragma once

#include <furi_hal_i2c.h>
#include <furi_hal_light.h>
#include <furi_hal_resources.h>
#include <furi_hal_spi.h>
#include <furi_hal_version.h>

#define furi_assert(value) (void)(value)

void furi_hal_init();

void furi_hal_delay_ms(float milliseconds);

void furi_hal_delay_us(float microseconds);
