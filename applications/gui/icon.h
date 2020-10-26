#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct IconData IconData;
typedef struct Icon Icon;

Icon* icon_alloc(const IconData* data);

void icon_free(Icon* icon);

uint8_t icon_get_width(Icon* icon);

uint8_t icon_get_height(Icon* icon);

bool icon_is_animated(Icon* icon);

void icon_start_animation(Icon* icon);

void icon_stop_animation(Icon* icon);
