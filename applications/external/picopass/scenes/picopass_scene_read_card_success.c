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

    FuriString* csn_str = furi_string_alloc_set("CSN:");
    FuriString* credential_str = furi_string_alloc();
    FuriString* wiegand_str = furi_string_alloc();
    FuriString* sio_str = furi_string_alloc();

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Send notification
    notification_message(picopass->notifications, &sequence_success);

    // Setup view
    PicopassBlock* AA1 = picopass->dev->dev_data.AA1;
    PicopassPacs* pacs = &picopass->dev->dev_data.pacs;
    Widget* widget = picopass->widget;

    uint8_t csn[PICOPASS_BLOCK_LEN] = {0};
    memcpy(csn, AA1[PICOPASS_CSN_BLOCK_INDEX].data, PICOPASS_BLOCK_LEN);
    for(uint8_t i = 0; i < PICOPASS_BLOCK_LEN; i++) {
        furi_string_cat_printf(csn_str, "%02X", csn[i]);
    }

    bool no_key = picopass_is_memset(pacs->key, 0x00, PICOPASS_BLOCK_LEN);
    bool empty =
        picopass_is_memset(AA1[PICOPASS_PACS_CFG_BLOCK_INDEX].data, 0xFF, PICOPASS_BLOCK_LEN);

    if(no_key) {
        furi_string_cat_printf(wiegand_str, "Read Failed");

        if(pacs->se_enabled) {
            furi_string_cat_printf(credential_str, "SE enabled");
        }

        widget_add_button_element(
            widget,
            GuiButtonTypeCenter,
            "Menu",
            picopass_scene_read_card_success_widget_callback,
            picopass);
    } else if(empty) {
        furi_string_cat_printf(wiegand_str, "Empty");
        widget_add_button_element(
            widget,
            GuiButtonTypeCenter,
            "Menu",
            picopass_scene_read_card_success_widget_callback,
            picopass);
    } else if(pacs->record.bitLength == 0 || pacs->record.bitLength == 255) {
        // Neither of these are valid.  Indicates the block was all 0x00 or all 0xff
        furi_string_cat_printf(wiegand_str, "Invalid PACS");

        if(pacs->se_enabled) {
            furi_string_cat_printf(credential_str, "SE enabled");
        }
        widget_add_button_element(
            widget,
            GuiButtonTypeCenter,
            "Menu",
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

        if(pacs->key) {
            if(pacs->sio) {
                furi_string_cat_printf(sio_str, " ");
            }
            furi_string_cat_printf(sio_str, "Key: ");

            uint8_t key[PICOPASS_BLOCK_LEN];
            memcpy(key, &pacs->key, PICOPASS_BLOCK_LEN);
            for(uint8_t i = 0; i < PICOPASS_BLOCK_LEN; i++) {
                furi_string_cat_printf(sio_str, "%02X", key[i]);
            }
        }

        widget_add_button_element(
            widget,
            GuiButtonTypeRight,
            "More",
            picopass_scene_read_card_success_widget_callback,
            picopass);
    }

    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Retry",
        picopass_scene_read_card_success_widget_callback,
        picopass);

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
        } else if(event.event == GuiButtonTypeCenter) {
            consumed = scene_manager_search_and_switch_to_another_scene(
                picopass->scene_manager, PicopassSceneStart);
        }
    }
    return consumed;
}

void picopass_scene_read_card_success_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear view
    widget_reset(picopass->widget);
}
