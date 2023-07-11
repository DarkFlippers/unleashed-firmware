#pragma once

typedef enum {
    // Scene events: Start menu
    CameraSuiteCustomEventStartUp,
    CameraSuiteCustomEventStartDown,
    CameraSuiteCustomEventStartLeft,
    CameraSuiteCustomEventStartRight,
    CameraSuiteCustomEventStartOk,
    CameraSuiteCustomEventStartBack,
    // Scene events: Camera style 1
    CameraSuiteCustomEventSceneStyle1Up,
    CameraSuiteCustomEventSceneStyle1Down,
    CameraSuiteCustomEventSceneStyle1Left,
    CameraSuiteCustomEventSceneStyle1Right,
    CameraSuiteCustomEventSceneStyle1Ok,
    CameraSuiteCustomEventSceneStyle1Back,
    // Scene events: Camera style 2
    CameraSuiteCustomEventSceneStyle2Up,
    CameraSuiteCustomEventSceneStyle2Down,
    CameraSuiteCustomEventSceneStyle2Left,
    CameraSuiteCustomEventSceneStyle2Right,
    CameraSuiteCustomEventSceneStyle2Ok,
    CameraSuiteCustomEventSceneStyle2Back,
    // Scene events: Guide
    CameraSuiteCustomEventSceneGuideUp,
    CameraSuiteCustomEventSceneGuideDown,
    CameraSuiteCustomEventSceneGuideLeft,
    CameraSuiteCustomEventSceneGuideRight,
    CameraSuiteCustomEventSceneGuideOk,
    CameraSuiteCustomEventSceneGuideBack,
} CameraSuiteCustomEvent;

enum CameraSuiteCustomEventType {
    // Reserve first 100 events for button types and indexes, starting from 0.
    CameraSuiteCustomEventMenuVoid,
    CameraSuiteCustomEventMenuSelected,
};

#pragma pack(push, 1)

typedef union {
    uint32_t packed_value;
    struct {
        uint16_t type;
        int16_t value;
    } content;
} CameraSuiteCustomEventMenu;

#pragma pack(pop)

static inline uint32_t camera_suite_custom_menu_event_pack(uint16_t type, int16_t value) {
    CameraSuiteCustomEventMenu event = {.content = {.type = type, .value = value}};
    return event.packed_value;
}

static inline void
    camera_suite_custom_menu_event_unpack(uint32_t packed_value, uint16_t* type, int16_t* value) {
    CameraSuiteCustomEventMenu event = {.packed_value = packed_value};
    if(type) *type = event.content.type;
    if(value) *value = event.content.value;
}

static inline uint16_t camera_suite_custom_menu_event_get_type(uint32_t packed_value) {
    uint16_t type;
    camera_suite_custom_menu_event_unpack(packed_value, &type, NULL);
    return type;
}

static inline int16_t camera_suite_custom_menu_event_get_value(uint32_t packed_value) {
    int16_t value;
    camera_suite_custom_menu_event_unpack(packed_value, NULL, &value);
    return value;
}
