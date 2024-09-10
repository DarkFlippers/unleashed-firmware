#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // reserve 100 for button presses, submenu selections, etc.
    DesktopSettingsCustomEventExit = 100,
    DesktopSettingsCustomEventDone,

    DesktopSettingsCustomEvent1stPinEntered,
    DesktopSettingsCustomEventPinsEqual,
    DesktopSettingsCustomEventPinsDifferent,

    DesktopSettingsCustomEventSetPin,
    DesktopSettingsCustomEventChangePin,
    DesktopSettingsCustomEventDisablePin,

    DesktopSettingsCustomEventSetDefault,
    DesktopSettingsCustomEventSetDummy,
} DesktopSettingsCustomEvent;

#ifdef __cplusplus
}
#endif
