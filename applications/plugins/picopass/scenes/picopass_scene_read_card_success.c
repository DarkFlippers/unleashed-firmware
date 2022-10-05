#include "../picopass_i.h"
#include <dolphin/dolphin.h>

void picopass_scene_read_card_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Picopass* picopass = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(picopass->view_dispatcher, result);
    }
}

void picopass_scene_read_card_success_on_enter(void* context) {
    Picopass* picopass = context;
    FuriString* credential_str;
    FuriString* wiegand_str;
    FuriString* sio_str;
    credential_str = furi_string_alloc();
    wiegand_str = furi_string_alloc();
    sio_str = furi_string_alloc();

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Send notification
    notification_message(picopass->notifications, &sequence_success);

    // Setup view
    PicopassPacs* pacs = &picopass->dev->dev_data.pacs;
    Widget* widget = picopass->widget;

    if(pacs->record.bitLength == 0) {
        furi_string_cat_printf(wiegand_str, "Read Failed");

        if(pacs->se_enabled) {
            furi_string_cat_printf(credential_str, "SE enabled");
        }

        widget_add_button_element(
            widget,
            GuiButtonTypeLeft,
            "Retry",
            picopass_scene_read_card_success_widget_callback,
            picopass);

    } else {
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

        if(pacs->sio) {
            furi_string_cat_printf(sio_str, "+SIO");
        }

        widget_add_button_element(
            widget,
            GuiButtonTypeLeft,
            "Retry",
            picopass_scene_read_card_success_widget_callback,
            picopass);

        widget_add_button_element(
            widget,
            GuiButtonTypeRight,
            "More",
            picopass_scene_read_card_success_widget_callback,
            picopass);
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
    widget_add_string_element(
        widget, 64, 42, AlignCenter, AlignCenter, FontSecondary, furi_string_get_cstr(sio_str));

    furi_string_free(credential_str);
    furi_string_free(wiegand_str);
    furi_string_free(sio_str);

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
}

bool picopass_scene_read_card_success_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(picopass->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            // Clear device name
            picopass_device_set_name(picopass->dev, "");
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneCardMenu);
            consumed = true;
        }
    }
    return consumed;
}

void picopass_scene_read_card_success_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear view
    widget_reset(picopass->widget);
}
