#include "lfrfid_app_scene_read_success.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

void LfRfidAppSceneReadSuccess::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string[0]);
    string_init(string[1]);
    string_init(string[2]);
    string_init(string[3]);

    auto container = app->view_controller.get<ContainerVM>();

    auto button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Left, "Retry");
    button->set_callback(app, LfRfidAppSceneReadSuccess::back_callback);

    button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Right, "More");
    button->set_callback(app, LfRfidAppSceneReadSuccess::more_callback);

    auto icon = container->add<IconElement>();
    icon->set_icon(3, 12, &I_RFIDBigChip_37x36);

    auto header = container->add<StringElement>();
    header->set_text(app->worker.key.get_type_text(), 89, 3, 0, AlignCenter);

    auto line_1_text = container->add<StringElement>();
    auto line_2l_text = container->add<StringElement>();
    auto line_2r_text = container->add<StringElement>();
    auto line_3_text = container->add<StringElement>();

    auto line_1_value = container->add<StringElement>();
    auto line_2l_value = container->add<StringElement>();
    auto line_2r_value = container->add<StringElement>();
    auto line_3_value = container->add<StringElement>();

    const uint8_t* data = app->worker.key.get_data();

    switch(app->worker.key.get_type()) {
    case LfrfidKeyType::KeyEM4100:
        line_1_text->set_text("HEX:", 65, 23, 0, AlignRight, AlignBottom, FontSecondary);
        line_2l_text->set_text("Mod:", 65, 35, 0, AlignRight, AlignBottom, FontSecondary);
        line_3_text->set_text("ID:", 65, 47, 0, AlignRight, AlignBottom, FontSecondary);

        for(uint8_t i = 0; i < app->worker.key.get_type_data_count(); i++) {
            string_cat_printf(string[0], "%02X", data[i]);
        }

        string_printf(string[1], "Manchester");
        string_printf(string[2], "%03u,%05u", data[2], (uint16_t)((data[3] << 8) | (data[4])));

        line_1_value->set_text(
            string_get_cstr(string[0]), 68, 23, 0, AlignLeft, AlignBottom, FontSecondary);
        line_2l_value->set_text(
            string_get_cstr(string[1]), 68, 35, 0, AlignLeft, AlignBottom, FontSecondary);
        line_3_value->set_text(
            string_get_cstr(string[2]), 68, 47, 0, AlignLeft, AlignBottom, FontSecondary);
        break;
    case LfrfidKeyType::KeyH10301:
    case LfrfidKeyType::KeyI40134:
        line_1_text->set_text("HEX:", 65, 23, 0, AlignRight, AlignBottom, FontSecondary);
        line_2l_text->set_text("FC:", 65, 35, 0, AlignRight, AlignBottom, FontSecondary);
        line_3_text->set_text("Card:", 65, 47, 0, AlignRight, AlignBottom, FontSecondary);

        for(uint8_t i = 0; i < app->worker.key.get_type_data_count(); i++) {
            string_cat_printf(string[0], "%02X", data[i]);
        }

        string_printf(string[1], "%u", data[0]);
        string_printf(string[2], "%u", (uint16_t)((data[1] << 8) | (data[2])));

        line_1_value->set_text(
            string_get_cstr(string[0]), 68, 23, 0, AlignLeft, AlignBottom, FontSecondary);
        line_2l_value->set_text(
            string_get_cstr(string[1]), 68, 35, 0, AlignLeft, AlignBottom, FontSecondary);
        line_3_value->set_text(
            string_get_cstr(string[2]), 68, 47, 0, AlignLeft, AlignBottom, FontSecondary);
        break;

    case LfrfidKeyType::KeyIoProxXSF:
        line_1_text->set_text("HEX:", 65, 23, 0, AlignRight, AlignBottom, FontSecondary);
        line_2l_text->set_text("FC:", 65, 35, 0, AlignRight, AlignBottom, FontSecondary);
        line_2r_text->set_text("VС:", 95, 35, 0, AlignRight, AlignBottom, FontSecondary);
        line_3_text->set_text("Card:", 65, 47, 0, AlignRight, AlignBottom, FontSecondary);

        for(uint8_t i = 0; i < app->worker.key.get_type_data_count(); i++) {
            string_cat_printf(string[0], "%02X", data[i]);
        }

        string_printf(string[1], "%u", data[0]);
        string_printf(string[2], "%u", (uint16_t)((data[2] << 8) | (data[3])));
        string_printf(string[3], "%u", data[1]);

        line_1_value->set_text(
            string_get_cstr(string[0]), 68, 23, 0, AlignLeft, AlignBottom, FontSecondary);
        line_2l_value->set_text(
            string_get_cstr(string[1]), 68, 35, 0, AlignLeft, AlignBottom, FontSecondary);
        line_2r_value->set_text(
            string_get_cstr(string[3]), 98, 35, 0, AlignLeft, AlignBottom, FontSecondary);
        line_3_value->set_text(
            string_get_cstr(string[2]), 68, 47, 0, AlignLeft, AlignBottom, FontSecondary);

        break;
    }

    app->view_controller.switch_to<ContainerVM>();

    notification_message_block(app->notification, &sequence_set_green_255);
}

bool LfRfidAppSceneReadSuccess::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Next) {
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::ReadKeyMenu);
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Retry) {
        app->scene_controller.switch_to_next_scene({LfRfidApp::SceneType::RetryConfirm});
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Back) {
        app->scene_controller.switch_to_next_scene({LfRfidApp::SceneType::ExitConfirm});
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneReadSuccess::on_exit(LfRfidApp* app) {
    notification_message_block(app->notification, &sequence_reset_green);
    app->view_controller.get<ContainerVM>()->clean();
    string_clear(string[0]);
    string_clear(string[1]);
    string_clear(string[2]);
}

void LfRfidAppSceneReadSuccess::back_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Retry;
    app->view_controller.send_event(&event);
}

void LfRfidAppSceneReadSuccess::more_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
