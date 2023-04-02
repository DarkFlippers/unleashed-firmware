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

    FuriString* csn_str = furi_string_alloc_set("CSN:");
    FuriString* credential_str = furi_string_alloc();
    FuriString* wiegand_str = furi_string_alloc();
    FuriString* sio_str = furi_string_alloc();

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup view
    PicopassBlock* AA1 = picopass->dev->dev_data.AA1;
    PicopassPacs* pacs = &picopass->dev->dev_data.pacs;
    Widget* widget = picopass->widget;

    uint8_t csn[PICOPASS_BLOCK_LEN] = {0};
    memcpy(csn, AA1[PICOPASS_CSN_BLOCK_INDEX].data, PICOPASS_BLOCK_LEN);
    for(uint8_t i = 0; i < PICOPASS_BLOCK_LEN; i++) {
        furi_string_cat_printf(csn_str, "%02X ", csn[i]);
    }

    if(pacs->record.bitLength == 0 || pacs->record.bitLength == 255) {
        // Neither of these are valid.  Indicates the block was all 0x00 or all 0xff
        furi_string_cat_printf(wiegand_str, "Invalid PACS");
    } else {
        size_t bytesLength = pacs->record.bitLength / 8;
        if(pacs->record.bitLength % 8 > 0) {
            // Add extra byte if there are bits remaining
            bytesLength++;
        }
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

        if(pacs->sio) {
            furi_string_cat_printf(sio_str, "+SIO");
        }
    }

    widget_add_string_element(
        widget, 64, 5, AlignCenter, AlignCenter, FontSecondary, furi_string_get_cstr(csn_str));
    widget_add_string_element(
        widget, 64, 20, AlignCenter, AlignCenter, FontPrimary, furi_string_get_cstr(wiegand_str));
    widget_add_string_element(
        widget,
        64,
        36,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        furi_string_get_cstr(credential_str));
    widget_add_string_element(
        widget, 64, 46, AlignCenter, AlignCenter, FontSecondary, furi_string_get_cstr(sio_str));

    furi_string_free(csn_str);
    furi_string_free(credential_str);
    furi_string_free(wiegand_str);
    furi_string_free(sio_str);

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
