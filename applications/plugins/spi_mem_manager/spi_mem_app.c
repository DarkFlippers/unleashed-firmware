#include <furi_hal.h>
#include "spi_mem_app_i.h"
#include "spi_mem_files.h"
#include "lib/spi/spi_mem_chip_i.h"

static bool spi_mem_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SPIMemApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool spi_mem_back_event_callback(void* context) {
    furi_assert(context);
    SPIMemApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

SPIMemApp* spi_mem_alloc(void) {
    SPIMemApp* instance = malloc(sizeof(SPIMemApp));

    instance->file_path = furi_string_alloc();
    instance->gui = furi_record_open(RECORD_GUI);
    instance->notifications = furi_record_open(RECORD_NOTIFICATION);
    instance->view_dispatcher = view_dispatcher_alloc();
    instance->scene_manager = scene_manager_alloc(&spi_mem_scene_handlers, instance);
    instance->submenu = submenu_alloc();
    instance->dialog_ex = dialog_ex_alloc();
    instance->popup = popup_alloc();
    instance->worker = spi_mem_worker_alloc();
    instance->dialogs = furi_record_open(RECORD_DIALOGS);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->widget = widget_alloc();
    instance->chip_info = malloc(sizeof(SPIMemChip));
    found_chips_init(instance->found_chips);
    instance->view_progress = spi_mem_view_progress_alloc();
    instance->view_detect = spi_mem_view_detect_alloc();
    instance->text_input = text_input_alloc();
    instance->mode = SPIMemModeUnknown;

    furi_string_set(instance->file_path, SPI_MEM_FILE_FOLDER);

    view_dispatcher_enable_queue(instance->view_dispatcher);
    view_dispatcher_set_event_callback_context(instance->view_dispatcher, instance);
    view_dispatcher_set_custom_event_callback(
        instance->view_dispatcher, spi_mem_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        instance->view_dispatcher, spi_mem_back_event_callback);
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(
        instance->view_dispatcher, SPIMemViewSubmenu, submenu_get_view(instance->submenu));
    view_dispatcher_add_view(
        instance->view_dispatcher, SPIMemViewDialogEx, dialog_ex_get_view(instance->dialog_ex));
    view_dispatcher_add_view(
        instance->view_dispatcher, SPIMemViewPopup, popup_get_view(instance->popup));
    view_dispatcher_add_view(
        instance->view_dispatcher, SPIMemViewWidget, widget_get_view(instance->widget));
    view_dispatcher_add_view(
        instance->view_dispatcher,
        SPIMemViewProgress,
        spi_mem_view_progress_get_view(instance->view_progress));
    view_dispatcher_add_view(
        instance->view_dispatcher,
        SPIMemViewDetect,
        spi_mem_view_detect_get_view(instance->view_detect));
    view_dispatcher_add_view(
        instance->view_dispatcher, SPIMemViewTextInput, text_input_get_view(instance->text_input));

    furi_hal_power_enable_otg();
    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_external);
    scene_manager_next_scene(instance->scene_manager, SPIMemSceneStart);
    return instance;
}

void spi_mem_free(SPIMemApp* instance) {
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewSubmenu);
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewDialogEx);
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewPopup);
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewWidget);
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewProgress);
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewDetect);
    view_dispatcher_remove_view(instance->view_dispatcher, SPIMemViewTextInput);
    spi_mem_view_progress_free(instance->view_progress);
    spi_mem_view_detect_free(instance->view_detect);
    submenu_free(instance->submenu);
    dialog_ex_free(instance->dialog_ex);
    popup_free(instance->popup);
    widget_free(instance->widget);
    text_input_free(instance->text_input);
    view_dispatcher_free(instance->view_dispatcher);
    scene_manager_free(instance->scene_manager);
    spi_mem_worker_free(instance->worker);
    free(instance->chip_info);
    found_chips_clear(instance->found_chips);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    furi_string_free(instance->file_path);
    furi_hal_spi_bus_handle_deinit(&furi_hal_spi_bus_handle_external);
    furi_hal_power_disable_otg();
    free(instance);
}

int32_t spi_mem_app(void* p) {
    UNUSED(p);
    SPIMemApp* instance = spi_mem_alloc();
    spi_mem_file_create_folder(instance);
    view_dispatcher_run(instance->view_dispatcher);
    spi_mem_free(instance);
    return 0;
}
