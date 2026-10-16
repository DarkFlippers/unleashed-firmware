#include "../t5577_app.h"

void t5577_app_scene_info_on_enter(void* context) {
    T5577App* app = context;

    t5577_app_describe(app);

    text_box_reset(app->text_box);
    text_box_set_font(app->text_box, TextBoxFontText);
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewTextBox);
}

bool t5577_app_scene_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void t5577_app_scene_info_on_exit(void* context) {
    T5577App* app = context;
    text_box_reset(app->text_box);
}
