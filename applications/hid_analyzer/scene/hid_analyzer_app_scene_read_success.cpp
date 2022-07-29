#include "hid_analyzer_app_scene_read_success.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

void HIDAppSceneReadSuccess::on_enter(HIDApp* app, bool /* need_restore */) {
    string_init(string[0]);
    string_init(string[1]);
    string_init(string[2]);

    auto container = app->view_controller.get<ContainerVM>();

    auto button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Left, "Retry");
    button->set_callback(app, HIDAppSceneReadSuccess::back_callback);

    auto icon = container->add<IconElement>();
    icon->set_icon(3, 12, &I_RFIDBigChip_37x36);

    auto header = container->add<StringElement>();
    header->set_text("HID", 89, 3, 0, AlignCenter);

    // auto line_1_text = container->add<StringElement>();
    auto line_2_text = container->add<StringElement>();
    // auto line_3_text = container->add<StringElement>();

    // auto line_1_value = container->add<StringElement>();
    auto line_2_value = container->add<StringElement>();
    // auto line_3_value = container->add<StringElement>();

    const uint8_t* data = app->worker.key.get_data();

    // line_1_text->set_text("Hi:", 65, 23, 0, AlignRight, AlignBottom, FontSecondary);
    line_2_text->set_text("Bit:", 65, 35, 0, AlignRight, AlignBottom, FontSecondary);
    // line_3_text->set_text("Bye:", 65, 47, 0, AlignRight, AlignBottom, FontSecondary);

    string_printf(string[1], "%u", data[0]);

    // line_1_value->set_text(
    // string_get_cstr(string[0]), 68, 23, 0, AlignLeft, AlignBottom, FontSecondary);
    line_2_value->set_text(
        string_get_cstr(string[1]), 68, 35, 0, AlignLeft, AlignBottom, FontSecondary);
    // line_3_value->set_text(
    // string_get_cstr(string[2]), 68, 47, 0, AlignLeft, AlignBottom, FontSecondary);

    app->view_controller.switch_to<ContainerVM>();

    notification_message_block(app->notification, &sequence_set_green_255);
}

bool HIDAppSceneReadSuccess::on_event(HIDApp* app, HIDApp::Event* event) {
    bool consumed = false;

    if(event->type == HIDApp::EventType::Retry) {
        app->scene_controller.search_and_switch_to_previous_scene({HIDApp::SceneType::Read});
        consumed = true;
    } else if(event->type == HIDApp::EventType::Back) {
        app->scene_controller.search_and_switch_to_previous_scene({HIDApp::SceneType::Read});
        consumed = true;
    }

    return consumed;
}

void HIDAppSceneReadSuccess::on_exit(HIDApp* app) {
    notification_message_block(app->notification, &sequence_reset_green);
    app->view_controller.get<ContainerVM>()->clean();
    string_clear(string[0]);
    string_clear(string[1]);
    string_clear(string[2]);
}

void HIDAppSceneReadSuccess::back_callback(void* context) {
    HIDApp* app = static_cast<HIDApp*>(context);
    HIDApp::Event event;
    event.type = HIDApp::EventType::Retry;
    app->view_controller.send_event(&event);
}
