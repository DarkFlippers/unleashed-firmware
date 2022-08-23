#include "lfrfid_app_scene_delete_confirm.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

void LfRfidAppSceneDeleteConfirm::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string_data);
    string_init(string_header);

    auto container = app->view_controller.get<ContainerVM>();

    auto button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Left, "Back");
    button->set_callback(app, LfRfidAppSceneDeleteConfirm::back_callback);

    button = container->add<ButtonElement>();
    button->set_type(ButtonElement::Type::Right, "Delete");
    button->set_callback(app, LfRfidAppSceneDeleteConfirm::delete_callback);

    auto line_1 = container->add<StringElement>();
    auto line_2 = container->add<StringElement>();
    auto line_3 = container->add<StringElement>();

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, data, size);
    for(uint8_t i = 0; i < MIN(size, (size_t)8); i++) {
        if(i != 0) {
            string_cat_printf(string_data, " ");
        }

        string_cat_printf(string_data, "%02X", data[i]);
    }
    free(data);

    string_printf(string_header, "Delete %s?", string_get_cstr(app->file_name));
    line_1->set_text(
        string_get_cstr(string_header), 64, 0, 128 - 2, AlignCenter, AlignTop, FontPrimary);
    line_2->set_text(
        string_get_cstr(string_data), 64, 19, 0, AlignCenter, AlignTop, FontSecondary);
    line_3->set_text(
        protocol_dict_get_name(app->dict, app->protocol_id),
        64,
        49,
        0,
        AlignCenter,
        AlignBottom,
        FontSecondary);

    app->view_controller.switch_to<ContainerVM>();
}

bool LfRfidAppSceneDeleteConfirm::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Next) {
        app->delete_key();
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::DeleteSuccess);
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Stay) {
        app->scene_controller.switch_to_previous_scene();
        consumed = true;
    } else if(event->type == LfRfidApp::EventType::Back) {
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneDeleteConfirm::on_exit(LfRfidApp* app) {
    app->view_controller.get<ContainerVM>()->clean();
    string_clear(string_data);
    string_clear(string_header);
}

void LfRfidAppSceneDeleteConfirm::back_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Stay;
    app->view_controller.send_event(&event);
}

void LfRfidAppSceneDeleteConfirm::delete_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
