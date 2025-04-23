/**
 * @file widget_element_i.h
 * GUI: internal Widget Element API
 */

#pragma once

#include <input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GuiButtonTypeLeft,
    GuiButtonTypeCenter,
    GuiButtonTypeRight,
} GuiButtonType;

typedef void (*ButtonCallback)(GuiButtonType result, InputType type, void* context);

#ifdef __cplusplus
}
#endif
