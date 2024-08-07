#include "archive_i.h"

bool archive_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ArchiveApp* archive = (ArchiveApp*)context;
    return scene_manager_handle_custom_event(archive->scene_manager, event);
}

bool archive_back_event_callback(void* context) {
    furi_assert(context);
    ArchiveApp* archive = (ArchiveApp*)context;
    return scene_manager_handle_back_event(archive->scene_manager);
}

ArchiveApp* archive_alloc(void) {
    ArchiveApp* archive = malloc(sizeof(ArchiveApp));

    archive->gui = furi_record_open(RECORD_GUI);
    archive->loader = furi_record_open(RECORD_LOADER);
    archive->text_input = text_input_alloc();
    archive->fav_move_str = furi_string_alloc();

    archive->view_dispatcher = view_dispatcher_alloc();
    archive->scene_manager = scene_manager_alloc(&archive_scene_handlers, archive);

    view_dispatcher_attach_to_gui(
        archive->view_dispatcher, archive->gui, ViewDispatcherTypeFullscreen);

    view_dispatcher_set_event_callback_context(archive->view_dispatcher, archive);
    view_dispatcher_set_custom_event_callback(
        archive->view_dispatcher, archive_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        archive->view_dispatcher, archive_back_event_callback);

    archive->browser = browser_alloc();

    view_dispatcher_add_view(
        archive->view_dispatcher, ArchiveViewBrowser, archive_browser_get_view(archive->browser));

    view_dispatcher_add_view(
        archive->view_dispatcher, ArchiveViewTextInput, text_input_get_view(archive->text_input));

    archive->widget = widget_alloc();
    view_dispatcher_add_view(
        archive->view_dispatcher, ArchiveViewWidget, widget_get_view(archive->widget));

    return archive;
}

void archive_free(ArchiveApp* archive) {
    furi_assert(archive);

    view_dispatcher_remove_view(archive->view_dispatcher, ArchiveViewBrowser);
    view_dispatcher_remove_view(archive->view_dispatcher, ArchiveViewTextInput);
    view_dispatcher_remove_view(archive->view_dispatcher, ArchiveViewWidget);
    widget_free(archive->widget);
    view_dispatcher_free(archive->view_dispatcher);
    scene_manager_free(archive->scene_manager);
    browser_free(archive->browser);
    furi_string_free(archive->fav_move_str);

    text_input_free(archive->text_input);

    furi_record_close(RECORD_LOADER);
    archive->loader = NULL;
    furi_record_close(RECORD_GUI);
    archive->gui = NULL;

    free(archive);
}

int32_t archive_app(void* p) {
    UNUSED(p);
    ArchiveApp* archive = archive_alloc();
    scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneBrowser);
    view_dispatcher_run(archive->view_dispatcher);
    archive_free(archive);

    return 0;
}
