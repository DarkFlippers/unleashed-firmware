#include <subghz/subghz_i.h>
#include "subghz_txrx_create_protocol_key.h"

#include <gui/modules/number_input.h>

/* Owned by this scene rather than the app: Add Manually is a plugin, so a view allocated here
 * costs nothing while it is unmapped - the frequency analyzer's view works the same way. */
static NumberInput* subghz_set_te_input = NULL;

static void subghz_scene_set_te_number_input_callback(void* context, int32_t number) {
    SubGhz* subghz = context;

    subghz->gen_info->data.te = (uint32_t)number;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventNumberInputDone);
}

void subghz_scene_set_te_on_enter(void* context) {
    SubGhz* subghz = context;
    furi_assert(subghz->gen_info->type == GenData);
    furi_assert(!subghz_set_te_input);

    /* The window belongs to the protocol, so it travels with the rest of its generation data
       - see the te_min/te_max comment in GenInfo. */
    furi_assert(subghz->gen_info->data.te_min < subghz->gen_info->data.te_max);

    subghz_set_te_input = number_input_alloc();
    number_input_set_header_text(subghz_set_te_input, "Enter TE in us");
    number_input_set_result_callback(
        subghz_set_te_input,
        subghz_scene_set_te_number_input_callback,
        subghz,
        (int32_t)subghz->gen_info->data.te,
        subghz->gen_info->data.te_min,
        subghz->gen_info->data.te_max);

    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdNumberInput,
        number_input_get_view(subghz_set_te_input));
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdNumberInput);
}

bool subghz_scene_set_te_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if((event.type != SceneManagerEventTypeCustom) ||
       (event.event != SubGhzCustomEventNumberInputDone)) {
        return false;
    }

    if(!subghz_txrx_gen_data_protocol_and_te(
           subghz->txrx,
           subghz->gen_info->mod,
           subghz->gen_info->freq,
           subghz->gen_info->data.name,
           subghz->gen_info->data.key,
           subghz->gen_info->data.bits,
           subghz->gen_info->data.te)) {
        furi_string_set(subghz->error_str, "Function requires\nan SD card with\nfresh databases.");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
        return true;
    }

    subghz_file_name_clear(subghz);
    scene_manager_set_scene_state(
        subghz->scene_manager, SubGhzSceneSaveName, SubGhzCustomEventManagerSet);
    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
    return true;
}

void subghz_scene_set_te_on_exit(void* context) {
    SubGhz* subghz = context;
    furi_assert(subghz_set_te_input);

    /* Park on an app-owned view before dropping ours: removing the current view leaves the
     * dispatcher with none, which stops the app's event loop for good. The next scene switches
     * to its own view right after. */
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdNumberInput);
    number_input_free(subghz_set_te_input);
    subghz_set_te_input = NULL;
}
