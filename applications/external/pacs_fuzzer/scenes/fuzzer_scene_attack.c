#include "../fuzzer_i.h"
#include "../helpers/fuzzer_custom_event.h"

#include "../helpers/protocol.h"
#include "../helpers/gui_const.h"

void fuzzer_scene_attack_callback(FuzzerCustomEvent event, void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fuzzer_scene_attack_on_enter(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;

    fuzzer_view_attack_set_callback(app->attack_view, fuzzer_scene_attack_callback, app);

    FuzzerProtocol proto = fuzzer_proto_items[app->fuzzer_state.proto_index];

    fuzzer_view_attack_reset_data(
        app->attack_view,
        fuzzer_attack_names[app->fuzzer_state.menu_index],
        proto.name,
        proto.data_size);
    fuzzer_view_attack_set_uid(app->attack_view, &proto.dict.val[0], false);

    view_dispatcher_switch_to_view(app->view_dispatcher, FuzzerViewIDAttack);
}

bool fuzzer_scene_attack_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FuzzerCustomEventViewAttackBack) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
        }
    }

    return consumed;
}

void fuzzer_scene_attack_on_exit(void* context) {
    // furi_assert(context);
    // PacsFuzzerApp* app = context;
    UNUSED(context);
}
