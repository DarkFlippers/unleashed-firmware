#include "lfrfid_app_scene_raw_success.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

void LfRfidAppSceneRawSuccess::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string_info);

    string_printf(string_info, "RAW RFID read success!\r\n");
    string_cat_printf(string_info, "Now you can analyze files\r\n");
    string_cat_printf(string_info, "Or send them to developers");

    auto container = app->view_controller.get<ContainerVM>();

    auto line = container->add<StringElement>();
    line->set_text(string_get_cstr(string_info), 0, 1, 0, AlignLeft, AlignTop, FontSecondary);

    auto button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Center, "OK");
    button->set_callback(app, LfRfidAppSceneRawSuccess::ok_callback);

    app->view_controller.switch_to<ContainerVM>();
}

bool LfRfidAppSceneRawSuccess::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;
    if(event->type == LfRfidApp::EventType::Next) {
        app->scene_controller.search_and_switch_to_previous_scene(
            {LfRfidApp::SceneType::ExtraActions});
        consumed = true;
    }
    return consumed;
}

void LfRfidAppSceneRawSuccess::on_exit(LfRfidApp* app) {
    app->view_controller.get<ContainerVM>()->clean();
    string_clear(string_info);
}

void LfRfidAppSceneRawSuccess::ok_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
