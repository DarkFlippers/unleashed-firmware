#include "../subghz_i.h" // IWYU pragma: keep
#include "../helpers/subghz_add_manually_plugin.h"

void subghz_scene_set_freq_on_enter(void* context) {
    subghz_add_manually_scene_on_enter(context, SubGhzAddManuallySceneSetFreq);
}

bool subghz_scene_set_freq_on_event(void* context, SceneManagerEvent event) {
    return subghz_add_manually_scene_on_event(context, SubGhzAddManuallySceneSetFreq, event);
}

void subghz_scene_set_freq_on_exit(void* context) {
    subghz_add_manually_scene_on_exit(context, SubGhzAddManuallySceneSetFreq);
}
