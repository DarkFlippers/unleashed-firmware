#include <subghz/subghz_i.h>
#include <subghz/helpers/subghz_add_manually_plugin.h>
#include <subghz/helpers/subghz_feature_plugin.h>

#include <flipper_application/flipper_application.h>

/* The scene bodies kept the names the app's scene table used to call directly, so they still
 * satisfy the prototypes subghz_scene.h generates; only the dispatch changed - the app's table now
 * reaches them through the shims in scenes/. */
static const SubGhzAddManuallyPlugin subghz_add_manually_plugin = {
    .scene =
        {
            [SubGhzAddManuallySceneSetType] =
                {subghz_scene_set_type_on_enter,
                 subghz_scene_set_type_on_event,
                 subghz_scene_set_type_on_exit},
            [SubGhzAddManuallySceneSetKey] =
                {subghz_scene_set_key_on_enter,
                 subghz_scene_set_key_on_event,
                 subghz_scene_set_key_on_exit},
            [SubGhzAddManuallySceneSetSerial] =
                {subghz_scene_set_serial_on_enter,
                 subghz_scene_set_serial_on_event,
                 subghz_scene_set_serial_on_exit},
            [SubGhzAddManuallySceneSetButton] =
                {subghz_scene_set_button_on_enter,
                 subghz_scene_set_button_on_event,
                 subghz_scene_set_button_on_exit},
            [SubGhzAddManuallySceneSetCounter] =
                {subghz_scene_set_counter_on_enter,
                 subghz_scene_set_counter_on_event,
                 subghz_scene_set_counter_on_exit},
            [SubGhzAddManuallySceneSetSeed] =
                {subghz_scene_set_seed_on_enter,
                 subghz_scene_set_seed_on_event,
                 subghz_scene_set_seed_on_exit},
        },
};

static const FlipperAppPluginDescriptor subghz_add_manually_plugin_descriptor = {
    .appid = SUBGHZ_ADD_MANUALLY_PLUGIN_APP_ID,
    .ep_api_version = SUBGHZ_FEATURE_PLUGIN_API_VERSION,
    .entry_point = &subghz_add_manually_plugin,
};

const FlipperAppPluginDescriptor* subghz_add_manually_ep(void) {
    return &subghz_add_manually_plugin_descriptor;
}
