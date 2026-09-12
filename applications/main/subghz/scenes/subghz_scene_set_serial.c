#include "../subghz_i.h" // IWYU pragma: keep
#include "../helpers/subghz_add_manually_plugin.h"

void subghz_scene_set_serial_on_enter(void* context) {
    subghz_add_manually_scene_on_enter(context, SubGhzAddManuallySceneSetSerial);
}

bool subghz_scene_set_serial_on_event(void* context, SceneManagerEvent event) {
    return subghz_add_manually_scene_on_event(context, SubGhzAddManuallySceneSetSerial, event);
}

void subghz_scene_set_serial_on_exit(void* context) {
    subghz_add_manually_scene_on_exit(context, SubGhzAddManuallySceneSetSerial);
}
