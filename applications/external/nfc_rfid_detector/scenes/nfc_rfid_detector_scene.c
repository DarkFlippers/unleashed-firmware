#include "../nfc_rfid_detector_app_i.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const nfc_rfid_detector_scene_on_enter_handlers[])(void*) = {
#include "nfc_rfid_detector_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const nfc_rfid_detector_scene_on_event_handlers[])(void* context, SceneManagerEvent event) =
    {
#include "nfc_rfid_detector_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const nfc_rfid_detector_scene_on_exit_handlers[])(void* context) = {
#include "nfc_rfid_detector_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers nfc_rfid_detector_scene_handlers = {
    .on_enter_handlers = nfc_rfid_detector_scene_on_enter_handlers,
    .on_event_handlers = nfc_rfid_detector_scene_on_event_handlers,
    .on_exit_handlers = nfc_rfid_detector_scene_on_exit_handlers,
    .scene_num = NfcRfidDetectorSceneNum,
};
