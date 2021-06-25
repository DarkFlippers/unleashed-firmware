#pragma once
#include <irda.h>
#include <gui/modules/dialog_ex.h>

class IrdaAppEvent {
public:
    enum class Type : uint8_t {
        Tick,
        Back,
        MenuSelected,
        DialogExSelected,
        NextScene,
        IrdaMessageReceived,
        TextEditDone,
        PopupTimer,
        ButtonPanelPressed,
        ButtonPanelPopupBackPressed,
    };

    union {
        int32_t menu_index;
        DialogExResult dialog_ex_result;
    } payload;

    Type type;
};

