#include "../lfrfid_i.h"

static void lfrfid_scene_save_fields_number_callback(void* context, int32_t number) {
    LfRfid* app = context;

    // the input refuses a value outside the field's range (Save is blocked), so it fits
    app->field_values[app->field_index] = number;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventNext);
}

static void lfrfid_scene_save_fields_show_field(LfRfid* app) {
    LfRfidManualFormatField field;
    furi_check(lfrfid_manual_format_field(app->manual_format, app->field_index, &field));

    // The input holds an int32, so the top of a wider field is only reachable as hex
    const int32_t min = field.min;
    const int32_t max = MIN(field.max, (uint64_t)INT32_MAX);

    lfrfid_text_store_set(app, "%s (%ld-%ld)", field.name, min, max);
    number_input_set_header_text(app->number_input, app->text_store);
    number_input_set_result_callback(
        app->number_input,
        lfrfid_scene_save_fields_number_callback,
        app,
        (int32_t)app->field_values[app->field_index],
        min,
        max);
}

void lfrfid_scene_save_fields_on_enter(void* context) {
    LfRfid* app = context;

    // the values are blanked by whoever enters this scene, so coming back from the name
    // entry keeps them
    app->field_index = 0;

    lfrfid_scene_save_fields_show_field(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewNumberInput);
}

bool lfrfid_scene_save_fields_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventNext) {
            consumed = true;
            const size_t count = lfrfid_manual_format_fields_count(app->manual_format);
            app->field_index++;

            if(app->field_index < count) {
                lfrfid_scene_save_fields_show_field(app);
            } else {
                // the input keeps every value within its field, so this cannot fail
                const size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
                furi_check(lfrfid_manual_format_encode(
                    app->manual_format, app->field_values, count, app->new_key_data, size));
                protocol_dict_set_data(app->dict, app->protocol_id, app->new_key_data, size);
                scene_manager_next_scene(scene_manager, LfRfidSceneSaveName);
            }
        }
    } else if(event.type == SceneManagerEventTypeBack && app->field_index > 0) {
        // back steps through the fields before it leaves the scene
        app->field_index--;
        lfrfid_scene_save_fields_show_field(app);
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_save_fields_on_exit(void* context) {
    UNUSED(context);
}
