#include "../sim800_app_i.h"
#include "../helpers/sim800_history.h"

typedef enum {
    SubmenuIndexCalls,
    SubmenuIndexSms,
    SubmenuIndexClear,
} SubmenuIndex;

#define SCENE_STATE_MENU 0
#define SCENE_STATE_TEXT 1

/** Decode any legacy raw UCS2 hex body lines saved before the Cyrillic plugin existed. */
static void sim800_history_decode_sms(Sim800App* app, FuriString* text) {
    if(!app->sms_plugin) return;

    FuriString* out = furi_string_alloc();
    FuriString* decoded = furi_string_alloc();
    FuriString* line = furi_string_alloc();
    const char* p = furi_string_get_cstr(text);

    while(*p != '\0') {
        const char* nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);

        if(len > 2 && p[0] == ' ' && p[1] == ' ') {
            furi_string_set_strn(line, p + 2, len - 2);
            app->sms_plugin->decode_text(furi_string_get_cstr(line), decoded);
            furi_string_cat_str(out, "  ");
            furi_string_cat(out, decoded);
        } else {
            furi_string_cat_printf(out, "%.*s", (int)len, p);
        }

        if(!nl) break;
        furi_string_push_back(out, '\n');
        p = nl + 1;
    }

    furi_string_set(text, out);
    furi_string_free(out);
    furi_string_free(decoded);
    furi_string_free(line);
}

static void sim800_history_show_menu(Sim800App* app) {
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "History");
    submenu_add_item(
        app->submenu, "Incoming calls", SubmenuIndexCalls, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Incoming SMS", SubmenuIndexSms, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Clear history", SubmenuIndexClear, sim800_submenu_callback, app);

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneHistory, SCENE_STATE_MENU);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

static void sim800_history_show_log(Sim800App* app, Sim800HistoryKind kind) {
    if(!sim800_history_read(app->text_box_store, (int32_t)kind)) {
        furi_string_set_str(app->text_box_store, "Nothing logged yet.");
    }

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneHistory, SCENE_STATE_TEXT);

    if(kind == Sim800HistorySms && sim800_sms_plugin_load(app)) {
        sim800_history_decode_sms(app, app->text_box_store);
        sim800_sms_view_set_callback(app->sms_view, NULL, NULL);
        sim800_sms_view_set_text(
            app->sms_view, "Incoming SMS", furi_string_get_cstr(app->text_box_store), true);
        view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSms);
        return;
    }

    text_box_reset(app->text_box);
    text_box_set_font(app->text_box, TextBoxFontText);
    // Newest entries are at the end, which is what you want to see first
    text_box_set_focus(app->text_box, TextBoxFocusEnd);
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextBox);
}

void sim800_scene_history_on_enter(void* context) {
    sim800_history_show_menu(context);
}

bool sim800_scene_history_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        if(scene_manager_get_scene_state(app->scene_manager, Sim800SceneHistory) ==
           SCENE_STATE_TEXT) {
            sim800_history_show_menu(app);
            return true;
        }
        return false;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case SubmenuIndexCalls:
        sim800_history_show_log(app, Sim800HistoryCall);
        return true;

    case SubmenuIndexSms:
        sim800_history_show_log(app, Sim800HistorySms);
        return true;

    case SubmenuIndexClear:
        sim800_history_clear();
        sim800_notify_result(app, true);
        return true;

    default:
        return false;
    }
}

void sim800_scene_history_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
    text_box_reset(app->text_box);
    sim800_sms_plugin_unload(app);
}
