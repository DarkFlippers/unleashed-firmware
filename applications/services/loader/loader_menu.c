#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/menu.h>
#include <gui/modules/submenu.h>
#include <assets_icons.h>
#include <applications.h>
#include <archive/helpers/archive_favorites.h>

#include "loader.h"
#include "loader_menu.h"
#include "loader_categories.h"
#include <storage/storage.h>
#include <gui/icon_i.h>
#include <strings.h>

#define TAG "LoaderMenu"

struct LoaderMenu {
    FuriThread* thread;
    void (*closed_cb)(void*);
    void* context;
    Menu* primary_menu;
    FuriString** categories;
    size_t category_count;
    ViewDispatcher* view_dispatcher;
    Submenu* category_menu;
    size_t active_category;
    Icon* fallback_icons;
    uint8_t* fallback_pixels;
    const uint8_t** fallback_frames;
};

typedef enum {
    LoaderMenuViewPrimary,
    LoaderMenuViewSettings,
    LoaderMenuViewCategory,
} LoaderMenuView;

static void loader_menu_start(const char* name) {
    Loader* loader = furi_record_open(RECORD_LOADER);
    loader_start_with_gui_error(loader, name, NULL);
    furi_record_close(RECORD_LOADER);
}

static void loader_menu_apps_callback(void* context, uint32_t index) {
    UNUSED(context);
    const char* name = FLIPPER_APPS[index].name;
    loader_menu_start(name);
}

static void loader_menu_external_apps_callback(void* context, uint32_t index) {
    UNUSED(context);
    const char* path = FLIPPER_EXTERNAL_APPS[index].name;
    loader_menu_start(path);
}

static void loader_menu_applications_callback(void* context, uint32_t index) {
    UNUSED(index);
    UNUSED(context);
    const char* name = LOADER_APPLICATIONS_NAME;
    loader_menu_start(name);
}

static void
    loader_menu_settings_menu_callback(void* context, InputType input_type, uint32_t index) {
    UNUSED(context);
    if(input_type == InputTypeShort) {
        loader_menu_start((const char*)index);
    } else if(input_type == InputTypeLong) {
        archive_favorites_handle_setting_pin_unpin((const char*)index, NULL);
    }
}

static void loader_menu_switch_to_settings(void* context, uint32_t index) {
    UNUSED(index);
    LoaderMenu* loader_menu = context;
    view_dispatcher_switch_to_view(loader_menu->view_dispatcher, LoaderMenuViewSettings);
}

static uint32_t loader_menu_switch_to_primary(void* context) {
    UNUSED(context);
    return LoaderMenuViewPrimary;
}

static uint32_t loader_menu_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

// Identity is the directory, NOT its display label. All menu-native tools are
// assigned by their existing path; multiple tools (USB, Sub-GHz) share one card.
static const char* loader_menu_internal_category(const char* name) {
    if(strcmp(name, "Sub-GHz") == 0) return "/ext/apps/Sub-GHz";
    return NULL;
}

static bool loader_menu_category_contains(const char* category, const char* app_path) {
    size_t n = strlen(category);
    return app_path && strncasecmp(category, app_path, n) == 0 && app_path[n] == '/' &&
           app_path[n + 1] && !strchr(app_path + n + 1, '/');
}

static const char* loader_menu_category_label(const char* path) {
    const char* name = strrchr(path, '/') + 1;
    return name;
}

static void loader_menu_category_launch_callback(void* context, uint32_t index) {
    LoaderMenu* menu = context;
    if(index < FLIPPER_APPS_COUNT) {
        loader_menu_start(FLIPPER_APPS[index].name);
    } else if(index - FLIPPER_APPS_COUNT < FLIPPER_EXTERNAL_APPS_COUNT) {
        loader_menu_start(FLIPPER_EXTERNAL_APPS[index - FLIPPER_APPS_COUNT].name);
    } else if(index == FLIPPER_APPS_COUNT + FLIPPER_EXTERNAL_APPS_COUNT &&
              menu->active_category < menu->category_count) {
        Loader* loader = furi_record_open(RECORD_LOADER);
        loader_start_with_gui_error(loader, LOADER_APPLICATIONS_NAME,
                                   furi_string_get_cstr(menu->categories[menu->active_category]));
        furi_record_close(RECORD_LOADER);
    }
}

static void loader_menu_category_callback(void* context, uint32_t index) {
    LoaderMenu* menu = context;
    if(index >= menu->category_count) return;
    menu->active_category = index;
    const char* path = furi_string_get_cstr(menu->categories[index]);
    submenu_reset(menu->category_menu);
    submenu_set_header(menu->category_menu, loader_menu_category_label(path));
    size_t native_count = 0;
    for(size_t i = 0; i < FLIPPER_APPS_COUNT; ++i) {
        const char* group = loader_menu_internal_category(FLIPPER_APPS[i].name);
        if(group && strcasecmp(path, group) == 0) {
            submenu_add_item(menu->category_menu, FLIPPER_APPS[i].name, i,
                             loader_menu_category_launch_callback, menu);
            ++native_count;
        }
    }
    for(size_t i = 0; i < FLIPPER_EXTERNAL_APPS_COUNT; ++i) {
        if(loader_menu_category_contains(path, FLIPPER_EXTERNAL_APPS[i].path)) {
            submenu_add_item(menu->category_menu, FLIPPER_EXTERNAL_APPS[i].name,
                             FLIPPER_APPS_COUNT + i, loader_menu_category_launch_callback, menu);
            ++native_count;
        }
    }
    // Zero-native categories go directly to their existing browser. Matched cards
    // offer the original instrument(s) plus the same folder, never copied files.
    if(native_count == 0) {
        loader_menu_category_launch_callback(menu, FLIPPER_APPS_COUNT + FLIPPER_EXTERNAL_APPS_COUNT);
        return;
    }
    submenu_add_item(menu->category_menu, "Applications", FLIPPER_APPS_COUNT + FLIPPER_EXTERNAL_APPS_COUNT,
                     loader_menu_category_launch_callback, menu);
    submenu_set_selected_item(menu->category_menu, 0);
    view_dispatcher_switch_to_view(menu->view_dispatcher, LoaderMenuViewCategory);
}

static size_t loader_menu_find_category(LoaderMenu* menu, const char* path) {
    for(size_t i = 0; i < menu->category_count; ++i) {
        if(strcasecmp(path, furi_string_get_cstr(menu->categories[i])) == 0) return i;
    }
    return SIZE_MAX;
}

static void loader_menu_add_category(LoaderMenu* menu, const char* path) {
    if(loader_menu_find_category(menu, path) != SIZE_MAX) return;
    menu->categories = realloc(menu->categories, (menu->category_count + 1) * sizeof(FuriString*));
    menu->categories[menu->category_count++] = furi_string_alloc_set(path);
}

// These wrappers retain the original pixel art; non-animated icons need a valid
// frame rate because Menu runs all icons through IconAnimation (including one frame).
static const Icon* loader_menu_category_icon(LoaderMenu* menu, size_t index) {
    const char* path = furi_string_get_cstr(menu->categories[index]);
    const char* name = strrchr(path, '/') + 1;
    for(size_t i = 0; i < FLIPPER_APPS_COUNT; ++i) {
        const char* group = loader_menu_internal_category(FLIPPER_APPS[i].name);
        if(group && strcasecmp(path, group) == 0) return FLIPPER_APPS[i].icon;
    }
    for(size_t i = 0; i < FLIPPER_EXTERNAL_APPS_COUNT; ++i) {
        if(loader_menu_category_contains(path, FLIPPER_EXTERNAL_APPS[i].path)) return FLIPPER_EXTERNAL_APPS[i].icon;
    }
    if(strcasecmp(name, "Bluetooth") == 0) return &A_HybridusBluetooth_14;
    if(strcasecmp(name, "Games") == 0) return &A_HybridusGames_14;
    if(strcasecmp(name, "Media") == 0) return &A_HybridusMedia_14;
    if(strcasecmp(name, "Scripts") == 0) return &A_HybridusScripts_14;
    {
        // User-added folders get distinct numbered pixel tiles, not the Apps icon.
        uint8_t* pixels = menu->fallback_pixels + index * 29;
        memset(pixels, 0, 29);
        for(size_t y = 0; y < 14; ++y) {
            uint16_t row = (y == 0 || y == 13) ? 0x3fff : 0x2001;
            if(y >= 4 && y <= 9) {
                for(size_t bit = 0; bit < 8; ++bit) {
                    if((index + 1) & (1U << bit)) row |= 1U << (bit + 3);
                }
            }
            pixels[1 + y * 2] = row & 0xff;
            pixels[2 + y * 2] = row >> 8;
        }
        menu->fallback_frames[index] = pixels;
        const Icon icon = {.width=14, .height=14, .frame_count=1, .frame_rate=1,
                           .frames=&menu->fallback_frames[index]};
        memcpy(&menu->fallback_icons[index], &icon, sizeof(icon));
    }
    return &menu->fallback_icons[index];
}

static int loader_menu_category_compare(const void* a, const void* b) {
    return furi_string_cmpi(*(FuriString* const*)a, furi_string_get_cstr(*(FuriString* const*)b));
}

static void loader_menu_read_categories(LoaderMenu* menu) {
    menu->categories = NULL;
    menu->category_count = 0;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    FileInfo info;
    char name[256];
    // Bounded memory for unusually large/user-created directory sets. Apps remains unlimited.
    if(storage_dir_open(dir, EXT_PATH("apps"))) {
        while(menu->category_count < 128 && storage_dir_read(dir, &info, name, sizeof(name))) {
            if(!file_info_is_dir(&info) || !loader_category_name_valid(name)) continue;
            FuriString* path = furi_string_alloc_printf("/ext/apps/%s", name);
            loader_menu_add_category(menu, furi_string_get_cstr(path));
            furi_string_free(path);
        }
    }
    storage_dir_close(dir);
    storage_file_free(dir);
    furi_record_close(RECORD_STORAGE);
    // Ensure the original instruments remain available even with a missing SD.
    // Scanned paths take precedence, preserving exact case on case-sensitive cards.
    for(size_t i = 0; i < FLIPPER_APPS_COUNT; ++i) {
        const char* path = loader_menu_internal_category(FLIPPER_APPS[i].name);
        if(path) loader_menu_add_category(menu, path);
    }
    for(size_t i = 0; i < FLIPPER_EXTERNAL_APPS_COUNT; ++i) {
        const char* path = FLIPPER_EXTERNAL_APPS[i].path;
        const char* slash = strrchr(path, '/');
        if(slash && strncmp(path, "/ext/apps/", 10) == 0) {
            FuriString* group = furi_string_alloc_printf("%.*s", (int)(slash - path), path);
            if(loader_category_path_valid(furi_string_get_cstr(group)))
                loader_menu_add_category(menu, furi_string_get_cstr(group));
            furi_string_free(group);
        }
    }
    if(menu->category_count > 1) qsort(menu->categories, menu->category_count,
                                     sizeof(FuriString*), loader_menu_category_compare);
}

static void loader_menu_build_menu(LoaderMenu* loader_menu) {
    Menu* menu = loader_menu->primary_menu;
    size_t i = 0;

    menu_add_item(
        menu,
        LOADER_APPLICATIONS_NAME,
        &A_Plugins_14,
        i++,
        loader_menu_applications_callback,
        loader_menu);

    // Original non-categorized internal/external entries remain reachable.
    for(i = 0; i < FLIPPER_APPS_COUNT; i++) {
        if(!loader_menu_internal_category(FLIPPER_APPS[i].name))
            menu_add_item(menu, FLIPPER_APPS[i].name, FLIPPER_APPS[i].icon, i,
                          loader_menu_apps_callback, loader_menu);
    }
    for(i = 0; i < FLIPPER_EXTERNAL_APPS_COUNT; i++) {
        bool grouped = false;
        for(size_t j = 0; j < loader_menu->category_count; ++j) {
            if(loader_menu_category_contains(furi_string_get_cstr(loader_menu->categories[j]),
                                             FLIPPER_EXTERNAL_APPS[i].path)) grouped = true;
        }
        if(!grouped) menu_add_item(menu, FLIPPER_EXTERNAL_APPS[i].name,
                                  FLIPPER_EXTERNAL_APPS[i].icon, i,
                                  loader_menu_external_apps_callback, loader_menu);
    }
    // The scanner already sorts directory names case-insensitively, just like
    // Apps. Display those names too: Tools must not disappear behind "Clock".
    for(size_t category = 0; category < loader_menu->category_count; ++category) {
        const char* path = furi_string_get_cstr(loader_menu->categories[category]);
        menu_add_item(menu, loader_menu_category_label(path), loader_menu_category_icon(loader_menu, category),
                      category, loader_menu_category_callback, loader_menu);
    }

    menu_add_item(
        menu, "Settings", &A_Settings_14, i++, loader_menu_switch_to_settings, loader_menu);
}

static void loader_menu_build_submenu(Submenu* submenu, LoaderMenu* loader_menu) {
    for(size_t i = 0; i < FLIPPER_EXTSETTINGS_APPS_COUNT; i++) {
        submenu_add_item_ex(
            submenu,
            FLIPPER_EXTSETTINGS_APPS[i].name,
            (uint32_t)FLIPPER_EXTSETTINGS_APPS[i].name,
            loader_menu_settings_menu_callback,
            loader_menu);
    }
    for(size_t i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
        submenu_add_item_ex(
            submenu,
            FLIPPER_SETTINGS_APPS[i].name,
            (uint32_t)FLIPPER_SETTINGS_APPS[i].name,
            loader_menu_settings_menu_callback,
            loader_menu);
    }
}

static int32_t loader_menu_thread(void* p) {
    LoaderMenu* loader_menu = p;
    furi_assert(loader_menu);

    Gui* gui = furi_record_open(RECORD_GUI);
    Submenu* settings_menu = submenu_alloc();
    loader_menu_build_submenu(settings_menu, loader_menu);

    loader_menu->view_dispatcher = view_dispatcher_alloc();

    View* primary_view = menu_get_view(loader_menu->primary_menu);
    view_set_previous_callback(primary_view, loader_menu_exit);
    view_dispatcher_add_view(loader_menu->view_dispatcher, LoaderMenuViewPrimary, primary_view);

    View* settings_view = submenu_get_view(settings_menu);
    view_set_previous_callback(settings_view, loader_menu_switch_to_primary);
    view_dispatcher_add_view(loader_menu->view_dispatcher, LoaderMenuViewSettings, settings_view);
    View* category_view = submenu_get_view(loader_menu->category_menu);
    view_set_previous_callback(category_view, loader_menu_switch_to_primary);
    view_dispatcher_add_view(loader_menu->view_dispatcher, LoaderMenuViewCategory, category_view);
    view_dispatcher_switch_to_view(loader_menu->view_dispatcher, LoaderMenuViewPrimary);

    view_dispatcher_attach_to_gui(loader_menu->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_run(loader_menu->view_dispatcher);

    if(loader_menu->closed_cb) {
        loader_menu->closed_cb(loader_menu->context);
    }

    view_dispatcher_remove_view(loader_menu->view_dispatcher, LoaderMenuViewCategory);
    view_dispatcher_remove_view(loader_menu->view_dispatcher, LoaderMenuViewPrimary);
    view_dispatcher_remove_view(loader_menu->view_dispatcher, LoaderMenuViewSettings);
    view_dispatcher_free(loader_menu->view_dispatcher);
    submenu_free(settings_menu);
    furi_record_close(RECORD_GUI);

    return 0;
}

LoaderMenu* loader_menu_alloc(void (*closed_cb)(void*), void* context, const MenuStyle* style) {
    LoaderMenu* loader_menu = malloc(sizeof(LoaderMenu));
    loader_menu->closed_cb = closed_cb;
    loader_menu->context = context;
    loader_menu->primary_menu = menu_alloc();
    menu_set_style(loader_menu->primary_menu, style);
    loader_menu_read_categories(loader_menu);
    loader_menu->category_menu = submenu_alloc();
    loader_menu->fallback_icons = malloc(loader_menu->category_count * sizeof(Icon));
    loader_menu->fallback_pixels = malloc(loader_menu->category_count * 29);
    loader_menu->fallback_frames = malloc(loader_menu->category_count * sizeof(uint8_t*));
    loader_menu_build_menu(loader_menu);
    loader_menu->thread = furi_thread_alloc_ex(TAG, 1024, loader_menu_thread, loader_menu);
    furi_thread_start(loader_menu->thread);
    return loader_menu;
}

void loader_menu_free(LoaderMenu* loader_menu) {
    furi_assert(loader_menu);
    furi_thread_join(loader_menu->thread);
    furi_thread_free(loader_menu->thread);
    menu_free(loader_menu->primary_menu);
    submenu_free(loader_menu->category_menu);
    free(loader_menu->fallback_icons);
    free(loader_menu->fallback_frames);
    free(loader_menu->fallback_pixels);
    for(size_t i = 0; i < loader_menu->category_count; ++i) {
        furi_string_free(loader_menu->categories[i]);
    }
    free(loader_menu->categories);
    free(loader_menu);
}

void loader_menu_set_style(LoaderMenu* loader_menu, const MenuStyle* style) {
    furi_assert(loader_menu);
    menu_set_style(loader_menu->primary_menu, style);
}
