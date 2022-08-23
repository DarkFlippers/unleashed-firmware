#include "lfrfid_app_scene_read_success.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

void LfRfidAppSceneReadSuccess::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string_info);
    string_init(string_header);

    string_init_printf(
        string_header,
        "%s[%s]",
        protocol_dict_get_name(app->dict, app->protocol_id),
        protocol_dict_get_manufacturer(app->dict, app->protocol_id));

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, data, size);
    for(uint8_t i = 0; i < size; i++) {
        if(i != 0) {
            string_cat_printf(string_info, " ");
        }

        if(i >= 9) {
            string_cat_printf(string_info, "...");
            break;
        } else {
            string_cat_printf(string_info, "%02X", data[i]);
        }
    }
    free(data);

    string_t render_data;
    string_init(render_data);
    protocol_dict_render_brief_data(app->dict, render_data, app->protocol_id);
    string_cat_printf(string_info, "\r\n%s", string_get_cstr(render_data));
    string_clear(render_data);

    auto container = app->view_controller.get<ContainerVM>();

    auto button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Left, "Retry");
    button->set_callback(app, LfRfidAppSceneReadSuccess::back_callback);

    button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Right, "More");
    button->set_callback(app, LfRfidAppSceneReadSuccess::more_callback);

    auto header = container->add<StringElement>();
    header->set_text(string_get_cstr(string_header), 0, 2, 0, AlignLeft, AlignTop, FontPrimary);

    auto text = container->add<StringElement>();
    text->set_text(string_get_cstr(string_info), 0, 16, 0, AlignLeft, AlignTop, FontSecondary);

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
    string_clear(string_info);
    string_clear(string_header);
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
