#include "avr_isp_app_i.h"

static bool avr_isp_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    AvrIspApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool avr_isp_app_back_event_callback(void* context) {
    furi_assert(context);
    AvrIspApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void avr_isp_app_tick_event_callback(void* context) {
    furi_assert(context);
    AvrIspApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

AvrIspApp* avr_isp_app_alloc() {
    AvrIspApp* app = malloc(sizeof(AvrIspApp));

    app->file_path = furi_string_alloc();
    furi_string_set(app->file_path, STORAGE_APP_DATA_PATH_PREFIX);
    app->error = AvrIspErrorNoError;

    // GUI
    app->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&avr_isp_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, avr_isp_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, avr_isp_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, avr_isp_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // SubMenu
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AvrIspViewSubmenu, submenu_get_view(app->submenu));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, AvrIspViewWidget, widget_get_view(app->widget));

    // Text Input
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, AvrIspViewTextInput, text_input_get_view(app->text_input));

    // Popup
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, AvrIspViewPopup, popup_get_view(app->popup));

    //Dialog
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    // Programmer view
    app->avr_isp_programmer_view = avr_isp_programmer_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        AvrIspViewProgrammer,
        avr_isp_programmer_view_get_view(app->avr_isp_programmer_view));

    // Reader view
    app->avr_isp_reader_view = avr_isp_reader_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        AvrIspViewReader,
        avr_isp_reader_view_get_view(app->avr_isp_reader_view));

    // Writer view
    app->avr_isp_writer_view = avr_isp_writer_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        AvrIspViewWriter,
        avr_isp_writer_view_get_view(app->avr_isp_writer_view));

    // Chip detect view
    app->avr_isp_chip_detect_view = avr_isp_chip_detect_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        AvrIspViewChipDetect,
        avr_isp_chip_detect_view_get_view(app->avr_isp_chip_detect_view));

    // Enable 5v power, multiple attempts to avoid issues with power chip protection false triggering
    uint8_t attempts = 0;
    while(!furi_hal_power_is_otg_enabled() && attempts++ < 5) {
        furi_hal_power_enable_otg();
        furi_delay_ms(10);
    }

    scene_manager_next_scene(app->scene_manager, AvrIspSceneStart);

    return app;
} //-V773

void avr_isp_app_free(AvrIspApp* app) {
    furi_assert(app);

    // Disable 5v power
    if(furi_hal_power_is_otg_enabled()) {
        furi_hal_power_disable_otg();
    }

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewSubmenu);
    submenu_free(app->submenu);

    //  Widget
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewWidget);
    widget_free(app->widget);

    // TextInput
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewTextInput);
    text_input_free(app->text_input);

    // Popup
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewPopup);
    popup_free(app->popup);

    //Dialog
    furi_record_close(RECORD_DIALOGS);

    // Programmer view
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewProgrammer);
    avr_isp_programmer_view_free(app->avr_isp_programmer_view);

    // Reader view
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewReader);
    avr_isp_reader_view_free(app->avr_isp_reader_view);

    // Writer view
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewWriter);
    avr_isp_writer_view_free(app->avr_isp_writer_view);

    // Chip detect view
    view_dispatcher_remove_view(app->view_dispatcher, AvrIspViewChipDetect);
    avr_isp_chip_detect_view_free(app->avr_isp_chip_detect_view);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Close records
    furi_record_close(RECORD_GUI);

    // Path strings
    furi_string_free(app->file_path);

    free(app);
}

int32_t avr_isp_app(void* p) {
    UNUSED(p);
    AvrIspApp* avr_isp_app = avr_isp_app_alloc();

    view_dispatcher_run(avr_isp_app->view_dispatcher);

    avr_isp_app_free(avr_isp_app);

    return 0;
}
