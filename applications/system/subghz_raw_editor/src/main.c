// Entry point and the start menu: Select file, Merge files, Config and About.

#include "app.h"

typedef enum {
    MenuViewSubmenu,
    MenuViewAbout,
    MenuViewConfig,
    MenuViewGapInput,
} MenuViewId;

typedef enum {
    MenuItemSelectFile,
    MenuItemMergeFiles,
    MenuItemConfig,
    MenuItemAbout,
} MenuItemId;

typedef enum {
    MenuActionNone = 0,
    MenuActionEdit,
    MenuActionMerge,
} MenuAction;

typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    VariableItemList* config_list;
    NumberInput* num_input;
    VariableItem* gap_item;
    VariableItem* repeat_item;
    int editing;
    Storage* storage;
    DialogsApp* dialogs;
    MenuAction action;
} Menu;

static void menu_submenu_cb(void* context, uint32_t index) {
    Menu* menu = context;
    if(index == MenuItemSelectFile) {
        menu->action = MenuActionEdit;
        view_dispatcher_stop(menu->view_dispatcher);
    } else if(index == MenuItemMergeFiles) {
        menu->action = MenuActionMerge;
        view_dispatcher_stop(menu->view_dispatcher);
    } else if(index == MenuItemConfig) {
        view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewConfig);
    } else if(index == MenuItemAbout) {
        view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewAbout);
    }
}

static uint32_t about_back_cb(void* context) {
    UNUSED(context);
    return MenuViewSubmenu;
}

static uint32_t config_back_cb(void* context) {
    UNUSED(context);
    return MenuViewSubmenu;
}

static void config_norm_changed_cb(VariableItem* item) {
    uint8_t idx = variable_item_get_current_value_index(item);
    g_normalize_jitter = (idx != 0);
    variable_item_set_current_value_text(item, idx ? "ON" : "OFF");
}

typedef enum {
    ConfigItemNormalize = 0,
    ConfigItemGap,
    ConfigItemRepeat,
} ConfigItem;

static void config_gap_sync_item(VariableItem* item) {
    // The gap uses a 3-slot stepper (down / hold / up) recentred on every change
    // so ±1 ms stepping works across the whole 1..1000 range, which is far
    // beyond a VariableItem's 255-value index limit.
    variable_item_set_current_value_index(item, 1);

    char buf[8];
    snprintf(buf, sizeof(buf), "%ld ms", (long)g_merge_gap_ms);
    variable_item_set_current_value_text(item, buf);
}

static void config_repeat_sync_item(VariableItem* item) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%ldx", (long)g_merge_repeat);
    variable_item_set_current_value_index(item, g_merge_repeat - MERGE_REPEAT_MIN);
    variable_item_set_current_value_text(item, buf);
}

static void config_gap_changed_cb(VariableItem* item) {
    uint8_t idx = variable_item_get_current_value_index(item);
    if(idx == 0 && g_merge_gap_ms > MERGE_GAP_MIN_MS)
        g_merge_gap_ms--;
    else if(idx == 2 && g_merge_gap_ms < MERGE_GAP_MAX_MS)
        g_merge_gap_ms++;

    config_gap_sync_item(item);
}

static void config_repeat_changed_cb(VariableItem* item) {
    g_merge_repeat = MERGE_REPEAT_MIN + variable_item_get_current_value_index(item);
    config_repeat_sync_item(item);
}

static void config_num_input_cb(void* context, int32_t v) {
    Menu* menu = context;

    if(menu->editing == ConfigItemGap) {
        if(v < MERGE_GAP_MIN_MS) v = MERGE_GAP_MIN_MS;
        if(v > MERGE_GAP_MAX_MS) v = MERGE_GAP_MAX_MS;

        g_merge_gap_ms = v;
        config_gap_sync_item(menu->gap_item);
    } else if(menu->editing == ConfigItemRepeat) {
        if(v < MERGE_REPEAT_MIN) v = MERGE_REPEAT_MIN;
        if(v > MERGE_REPEAT_MAX) v = MERGE_REPEAT_MAX;

        g_merge_repeat = v;
        config_repeat_sync_item(menu->repeat_item);
    }

    view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewConfig);
}

static uint32_t config_num_input_back_cb(void* context) {
    UNUSED(context);
    return MenuViewConfig;
}

// OK on a numeric config item opens a number picker.
static void config_enter_cb(void* context, uint32_t index) {
    Menu* menu = context;

    const char* header;
    int32_t current, min_val, max_val;
    if(index == ConfigItemGap) {
        header = "Merge gap [ms]";
        current = g_merge_gap_ms;
        min_val = MERGE_GAP_MIN_MS;
        max_val = MERGE_GAP_MAX_MS;
    } else if(index == ConfigItemRepeat) {
        header = "Repeat each (max: " STR(MERGE_REPEAT_MAX) ")";
        current = g_merge_repeat;
        min_val = MERGE_REPEAT_MIN;
        max_val = MERGE_REPEAT_MAX;
    } else {
        return; // non-numeric item (e.g. Normalize jitter)
    }

    menu->editing = index;
    number_input_set_header_text(menu->num_input, header);
    number_input_set_result_callback(
        menu->num_input, config_num_input_cb, menu, current, min_val, max_val);
    view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewGapInput);
}

static void menu_build_config(Menu* menu) {
    VariableItem* it = variable_item_list_add(
        menu->config_list, "Normalize jitter", 2, config_norm_changed_cb, menu);
    variable_item_set_current_value_index(it, g_normalize_jitter ? 1 : 0);
    variable_item_set_current_value_text(it, g_normalize_jitter ? "ON" : "OFF");

    menu->gap_item =
        variable_item_list_add(menu->config_list, "Merge gap", 3, config_gap_changed_cb, menu);
    config_gap_sync_item(menu->gap_item);

    menu->repeat_item = variable_item_list_add(
        menu->config_list,
        "Repeat",
        MERGE_REPEAT_MAX - MERGE_REPEAT_MIN + 1,
        config_repeat_changed_cb,
        menu);
    config_repeat_sync_item(menu->repeat_item);

    // Fixed by the SDK the app was built with, so set apart under a line
    const char* state = APP_CUSTOM_FW_FEATURES ? "ON" : "OFF";
    variable_item_list_add(menu->config_list, "------ Read-only ------", 1, NULL, NULL);
    it = variable_item_list_add(menu->config_list, "Custom buttons", 1, NULL, NULL);
    variable_item_set_current_value_text(it, state);
    it = variable_item_list_add(menu->config_list, "Keep counter", 1, NULL, NULL);
    variable_item_set_current_value_text(it, state);

    variable_item_list_set_enter_callback(menu->config_list, config_enter_cb, menu);
}

static uint32_t submenu_back_cb(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static void menu_build_about(Menu* menu) {
    int title_h = FONT_SIZE_PX * 2;
    widget_add_text_box_element(
        menu->widget,
        0,
        0,
        SCREEN_W_PX,
        title_h,
        AlignCenter,
        AlignBottom,
        "\e#\e!     Sub-GHz RAW Edit     \e!\n",
        false);

    int scroll_element_offset = title_h + GAP_PX;
    widget_add_text_scroll_element(
        menu->widget,
        0,
        scroll_element_offset,
        SCREEN_W_PX,
        SCREEN_H_PX - scroll_element_offset,
        "\e#Version " APP_VERSION "\n"
        "Enjoyed? Leave a star:\n"
        "" APP_REPO "\n"
        "\n"
        "A tiny on-device waveform\n"
        "editor for Flipper Zero that\n"
        "trims RAW .sub captures\n"
        "down to just the part\n"
        "you care about.\n"
        "\n"
        "Full documentation available on the GitHub.");
}

int32_t subghz_raw_edit_app(void* p) {
    // Launched with a file (e.g. Sub-GHz -> More -> Edit): open it directly.
    const char* file = p;
    bool direct = file && *file;

    Menu* menu = safe_malloc(sizeof(Menu));
    if(!menu) return 0;

    memset(menu, 0, sizeof(Menu));

    menu->storage = furi_record_open(RECORD_STORAGE);
    menu->dialogs = furi_record_open(RECORD_DIALOGS);
    Gui* gui = furi_record_open(RECORD_GUI);

    menu->view_dispatcher = view_dispatcher_alloc();
    menu->submenu = submenu_alloc();
    menu->widget = widget_alloc();
    menu->config_list = variable_item_list_alloc();
    menu->num_input = number_input_alloc();

    submenu_set_header(menu->submenu, APP_NAME);
    submenu_add_item(menu->submenu, "Select .sub file", MenuItemSelectFile, menu_submenu_cb, menu);
    submenu_add_item(menu->submenu, "Merge .sub files", MenuItemMergeFiles, menu_submenu_cb, menu);
    submenu_add_item(menu->submenu, "Config", MenuItemConfig, menu_submenu_cb, menu);
    submenu_add_item(menu->submenu, "About", MenuItemAbout, menu_submenu_cb, menu);

    menu_build_about(menu);
    menu_build_config(menu);

    view_set_previous_callback(submenu_get_view(menu->submenu), submenu_back_cb);
    view_set_previous_callback(widget_get_view(menu->widget), about_back_cb);
    view_set_previous_callback(variable_item_list_get_view(menu->config_list), config_back_cb);
    view_set_previous_callback(number_input_get_view(menu->num_input), config_num_input_back_cb);

    view_dispatcher_attach_to_gui(menu->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(
        menu->view_dispatcher, MenuViewSubmenu, submenu_get_view(menu->submenu));
    view_dispatcher_add_view(menu->view_dispatcher, MenuViewAbout, widget_get_view(menu->widget));
    view_dispatcher_add_view(
        menu->view_dispatcher, MenuViewConfig, variable_item_list_get_view(menu->config_list));
    view_dispatcher_add_view(
        menu->view_dispatcher, MenuViewGapInput, number_input_get_view(menu->num_input));

    if(direct) run_editor(menu->storage, menu->dialogs, file);

    bool running = !direct;
    while(running) {
        menu->action = MenuActionNone;
        view_dispatcher_switch_to_view(menu->view_dispatcher, MenuViewSubmenu);
        view_dispatcher_run(menu->view_dispatcher);

        if(menu->action == MenuActionEdit) {
            run_editor(menu->storage, menu->dialogs, NULL);
        } else if(menu->action == MenuActionMerge) {
            run_merge(menu->storage, menu->dialogs);
        } else {
            running = false;
        }
    }

    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewSubmenu);
    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewAbout);
    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewConfig);
    view_dispatcher_remove_view(menu->view_dispatcher, MenuViewGapInput);
    submenu_free(menu->submenu);
    widget_free(menu->widget);
    variable_item_list_free(menu->config_list);
    number_input_free(menu->num_input);
    view_dispatcher_free(menu->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);

    free(menu);
    return 0;
}
