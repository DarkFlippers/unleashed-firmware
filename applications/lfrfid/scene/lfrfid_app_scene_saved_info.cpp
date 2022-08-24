#include "lfrfid_app_scene_saved_info.h"
#include "../view/elements/button_element.h"
#include "../view/elements/icon_element.h"
#include "../view/elements/string_element.h"

void LfRfidAppSceneSavedInfo::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string_info);

    string_printf(
        string_info,
        "%s [%s]\r\n",
        string_get_cstr(app->file_name),
        protocol_dict_get_name(app->dict, app->protocol_id));

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    uint8_t* data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, data, size);
    for(uint8_t i = 0; i < size; i++) {
        if(i != 0) {
            string_cat_printf(string_info, " ");
        }

        string_cat_printf(string_info, "%02X", data[i]);
    }
    free(data);

    string_t render_data;
    string_init(render_data);
    protocol_dict_render_data(app->dict, render_data, app->protocol_id);
    string_cat_printf(string_info, "\r\n%s", string_get_cstr(render_data));
    string_clear(render_data);

    auto container = app->view_controller.get<ContainerVM>();

    auto line_1 = container->add<StringElement>();
    line_1->set_text(string_get_cstr(string_info), 0, 1, 0, AlignLeft, AlignTop, FontSecondary);

    app->view_controller.switch_to<ContainerVM>();
}

bool LfRfidAppSceneSavedInfo::on_event(LfRfidApp* /* app */, LfRfidApp::Event* /* event */) {
    return false;
}

void LfRfidAppSceneSavedInfo::on_exit(LfRfidApp* app) {
    app->view_controller.get<ContainerVM>()->clean();
    string_clear(string_info);
}
