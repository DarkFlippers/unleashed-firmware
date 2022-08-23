#include "lfrfid_app_scene_raw_info.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

static void ok_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}

static void back_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Back;
    app->view_controller.send_event(&event);
}

void LfRfidAppSceneRawInfo::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string_info);

    auto container = app->view_controller.get<ContainerVM>();

    bool sd_exist = storage_sd_status(app->storage) == FSE_OK;
    if(!sd_exist) {
        auto icon = container->add<IconElement>();
        icon->set_icon(0, 0, &I_SDQuestion_35x43);
        auto line = container->add<StringElement>();
        line->set_text(
            "No SD card found.\nThis function will not\nwork without\nSD card.",
            81,
            4,
            0,
            AlignCenter,
            AlignTop,
            FontSecondary);

        auto button = container->add<ButtonElement>();
        button->set_type(ButtonElement::Type::Left, "Back");
        button->set_callback(app, back_callback);
    } else {
        string_printf(
            string_info,
            "RAW RFID data reader\r\n"
            "1) Put the Flipper on your card\r\n"
            "2) Press OK\r\n"
            "3) Wait until data is read");

        auto line = container->add<StringElement>();
        line->set_text(string_get_cstr(string_info), 0, 1, 0, AlignLeft, AlignTop, FontSecondary);

        auto button = container->add<ButtonElement>();
        button->set_type(ButtonElement::Type::Center, "OK");
        button->set_callback(app, ok_callback);
    }

    app->view_controller.switch_to<ContainerVM>();
}

bool LfRfidAppSceneRawInfo::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;
    if(event->type == LfRfidApp::EventType::Next) {
        app->scene_controller.switch_to_scene({LfRfidApp::SceneType::RawRead});
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Back) {
        app->scene_controller.search_and_switch_to_previous_scene(
            {LfRfidApp::SceneType::ExtraActions});
        consumed = true;
    }
    return consumed;
}

void LfRfidAppSceneRawInfo::on_exit(LfRfidApp* app) {
    app->view_controller.get<ContainerVM>()->clean();
    string_clear(string_info);
}
