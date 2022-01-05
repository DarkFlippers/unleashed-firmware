#pragma once
#include <stdint.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/widget.h>

class iButtonApp;

class iButtonEvent {
public:
    // events enum
    enum class Type : uint8_t {
        EventTypeTick,
        EventTypeBack,
        EventTypeMenuSelected,
        EventTypeDialogResult,
        EventTypeTextEditResult,
        EventTypeByteEditResult,
        EventTypeWidgetButtonResult,
    };

    // payload
    union {
        uint32_t menu_index;
        DialogExResult dialog_result;
        GuiButtonType widget_button_result;
    } payload;

    // event type
    Type type;
};
