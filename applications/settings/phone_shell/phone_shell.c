#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>
#include <toolbox/hybridus_color.h>

static const char* const color_names[] = {"Real", "Black", "White", "Transparent"};
static void phone_shell_changed(VariableItem* item) {

    uint8_t value = variable_item_get_current_value_index(item);
    if(!hybridus_color_set(value)) {
        variable_item_set_current_value_index(item, hybridus_color_get());
        variable_item_set_current_value_text(item, "Save failed");
        return;
    }
    variable_item_set_current_value_text(item, color_names[value]);
}
static uint32_t phone_shell_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}
int32_t phone_shell_app(void* context) {
    UNUSED(context);
    Gui* gui = furi_record_open(RECORD_GUI);
    VariableItemList* list = variable_item_list_alloc();
    VariableItem* item = variable_item_list_add(list, "Color", 4, phone_shell_changed, NULL);
    uint8_t color = hybridus_color_get();
    variable_item_set_current_value_index(item, color);
    variable_item_set_current_value_text(item, color_names[color]);
    View* view = variable_item_list_get_view(list);
    view_set_previous_callback(view, phone_shell_exit);
    ViewDispatcher* dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(dispatcher, 0, view);
    view_dispatcher_switch_to_view(dispatcher, 0);
    view_dispatcher_run(dispatcher);
    view_dispatcher_remove_view(dispatcher, 0);
    view_dispatcher_free(dispatcher);
    variable_item_list_free(list);
    furi_record_close(RECORD_GUI);
    return 0;
}
