#pragma once
#include <stdint.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/widget.h>
#include <one_wire/ibutton/ibutton_worker.h>

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
        EventTypeWorkerEmulated,
        EventTypeWorkerRead,
        EventTypeWorkerWrite,
    };

    // payload
    union {
        uint32_t menu_index;
        DialogExResult dialog_result;
        GuiButtonType widget_button_result;
        iButtonWorkerWriteResult worker_write_result;
    } payload;

    // event type
    Type type;
};
