#pragma once
#include <stdint.h>

typedef struct GuiElement GuiElement;

typedef enum {
    GuiButtonTypeLeft,
    GuiButtonTypeCenter,
    GuiButtonTypeRight,
} GuiButtonType;

typedef void (*ButtonCallback)(GuiButtonType button, void* context);

/** Allocate Button Element
 * @param button_type GuiButtonType instance
 * @param text text on allocated button
 * @param callback ButtonCallback instance
 * @param context pointer to context
 */
GuiElement* gui_button_create(
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);
