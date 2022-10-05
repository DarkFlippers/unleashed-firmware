#include "../picopass_i.h"
#include <dolphin/dolphin.h>

void picopass_scene_device_info_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Picopass* picopass = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(picopass->view_dispatcher, result);
    }
}

void picopass_scene_device_info_on_enter(void* context) {
    Picopass* picopass = context;

    FuriString* credential_str;
    FuriString* wiegand_str;
    credential_str = furi_string_alloc();
    wiegand_str = furi_string_alloc();

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup view
    PicopassPacs* pacs = &picopass->dev->dev_data.pacs;
    Widget* widget = picopass->widget;

    size_t bytesLength = 1 + pacs->record.bitLength / 8;
    furi_string_set(credential_str, "");
    for(uint8_t i = PICOPASS_BLOCK_LEN - bytesLength; i < PICOPASS_BLOCK_LEN; i++) {
        furi_string_cat_printf(credential_str, " %02X", pacs->credential[i]);
    }

    if(pacs->record.valid) {
        furi_string_cat_printf(
            wiegand_str, "FC: %u CN: %u", pacs->record.FacilityCode, pacs->record.CardNumber);
    } else {
        furi_string_cat_printf(wiegand_str, "%d bits", pacs->record.bitLength);
    }

    widget_add_string_element(
        widget, 64, 12, AlignCenter, AlignCenter, FontPrimary, furi_string_get_cstr(wiegand_str));
    widget_add_string_element(
        widget,
        64,
        32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        furi_string_get_cstr(credential_str));

    furi_string_free(credential_str);
    furi_string_free(wiegand_str);

    widget_add_button_element(
        picopass->widget,
        GuiButtonTypeLeft,
        "Back",
        picopass_scene_device_info_widget_callback,
        picopass);

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
}

bool picopass_scene_device_info_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(picopass->scene_manager);
        } else if(event.event == PicopassCustomEventViewExit) {
            view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
        consumed = true;
    }
    return consumed;
}

void picopass_scene_device_info_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear views
    widget_reset(picopass->widget);
}
