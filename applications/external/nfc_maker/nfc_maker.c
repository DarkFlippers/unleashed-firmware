#include "nfc_maker.h"

static bool nfc_maker_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    NfcMaker* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool nfc_maker_back_event_callback(void* context) {
    furi_assert(context);
    NfcMaker* app = context;

    return scene_manager_handle_back_event(app->scene_manager);
}

NfcMaker* nfc_maker_alloc() {
    NfcMaker* app = malloc(sizeof(NfcMaker));
    app->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher and Scene Manager
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&nfc_maker_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, nfc_maker_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, nfc_maker_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Gui Modules
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcMakerViewSubmenu, submenu_get_view(app->submenu));

    app->text_input = nfc_maker_text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        NfcMakerViewTextInput,
        nfc_maker_text_input_get_view(app->text_input));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcMakerViewByteInput, byte_input_get_view(app->byte_input));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, NfcMakerViewPopup, popup_get_view(app->popup));

    return app;
}

void nfc_maker_free(NfcMaker* app) {
    furi_assert(app);

    // Gui modules
    view_dispatcher_remove_view(app->view_dispatcher, NfcMakerViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, NfcMakerViewTextInput);
    nfc_maker_text_input_free(app->text_input);
    view_dispatcher_remove_view(app->view_dispatcher, NfcMakerViewByteInput);
    byte_input_free(app->byte_input);
    view_dispatcher_remove_view(app->view_dispatcher, NfcMakerViewPopup);
    popup_free(app->popup);

    // View Dispatcher and Scene Manager
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Records
    furi_record_close(RECORD_GUI);
    free(app);
}

extern int32_t nfc_maker(void* p) {
    UNUSED(p);
    NfcMaker* app = nfc_maker_alloc();
    scene_manager_set_scene_state(app->scene_manager, NfcMakerSceneStart, NfcMakerSceneHttps);
    scene_manager_next_scene(app->scene_manager, NfcMakerSceneStart);
    view_dispatcher_run(app->view_dispatcher);
    nfc_maker_free(app);
    return 0;
}
