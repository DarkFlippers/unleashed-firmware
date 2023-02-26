#include "../picopass_i.h"
#include <dolphin/dolphin.h>

void picopass_scene_read_factory_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Picopass* picopass = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(picopass->view_dispatcher, result);
    }
}

void picopass_scene_read_factory_success_on_enter(void* context) {
    Picopass* picopass = context;
    FuriString* title = furi_string_alloc_set("Factory Default");
    FuriString* subtitle = furi_string_alloc_set("");

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Send notification
    notification_message(picopass->notifications, &sequence_success);

    // Setup view
    Widget* widget = picopass->widget;
    //PicopassPacs* pacs = &picopass->dev->dev_data.pacs;
    PicopassBlock* AA1 = picopass->dev->dev_data.AA1;

    uint8_t* configBlock = AA1[PICOPASS_CONFIG_BLOCK_INDEX].data;
    uint8_t fuses = configBlock[7];

    if((fuses & 0x80) == 0x80) {
        furi_string_cat_printf(subtitle, "Personalization mode");
    } else {
        furi_string_cat_printf(subtitle, "Application mode");
    }

    widget_add_button_element(
        widget,
        GuiButtonTypeCenter,
        "Write Standard iClass Key",
        picopass_scene_read_factory_success_widget_callback,
        picopass);

    widget_add_string_element(
        widget, 64, 5, AlignCenter, AlignCenter, FontSecondary, furi_string_get_cstr(title));
    widget_add_string_element(
        widget, 64, 20, AlignCenter, AlignCenter, FontPrimary, furi_string_get_cstr(subtitle));

    furi_string_free(title);
    furi_string_free(subtitle);

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
}

bool picopass_scene_read_factory_success_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(picopass->scene_manager);
        } else if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneWriteKey);
            consumed = true;
        }
    }
    return consumed;
}

void picopass_scene_read_factory_success_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear view
    widget_reset(picopass->widget);
}
