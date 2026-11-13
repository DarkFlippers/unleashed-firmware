#include "../subghz_i.h" // IWYU pragma: keep
#include "../helpers/subghz_add_manually_plugin.h"

void subghz_scene_set_gt_on_enter(void* context) {
    subghz_add_manually_scene_on_enter(context, SubGhzAddManuallySceneSetGt);
}

bool subghz_scene_set_gt_on_event(void* context, SceneManagerEvent event) {
    return subghz_add_manually_scene_on_event(context, SubGhzAddManuallySceneSetGt, event);
}

void subghz_scene_set_gt_on_exit(void* context) {
    subghz_add_manually_scene_on_exit(context, SubGhzAddManuallySceneSetGt);
}
