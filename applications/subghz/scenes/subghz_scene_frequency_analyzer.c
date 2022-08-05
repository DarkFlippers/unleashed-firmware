#include "../subghz_i.h"
#include "../views/subghz_frequency_analyzer.h"
#include <dolphin/dolphin.h>

void subghz_scene_frequency_analyzer_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_frequency_analyzer_on_enter(void* context) {
    SubGhz* subghz = context;
    DOLPHIN_DEED(DolphinDeedSubGhzFrequencyAnalyzer);
    subghz_frequency_analyzer_set_callback(
        subghz->subghz_frequency_analyzer, subghz_scene_frequency_analyzer_callback, subghz);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
}

bool subghz_scene_frequency_analyzer_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void subghz_scene_frequency_analyzer_on_exit(void* context) {
    SubGhz* subghz = context;
    notification_message(subghz->notifications, &sequence_reset_rgb);
}
