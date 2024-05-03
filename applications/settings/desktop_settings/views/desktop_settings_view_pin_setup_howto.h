#pragma once

#include <gui/view.h>

typedef struct DesktopSettingsViewPinSetupHowto DesktopSettingsViewPinSetupHowto;

typedef void (*DesktopSettingsViewPinSetupHowtoDoneCallback)(void*);

void desktop_settings_view_pin_setup_howto_set_callback(
    DesktopSettingsViewPinSetupHowto* instance,
    DesktopSettingsViewPinSetupHowtoDoneCallback callback,
    void* context);
DesktopSettingsViewPinSetupHowto* desktop_settings_view_pin_setup_howto_alloc(void);
void desktop_settings_view_pin_setup_howto_free(DesktopSettingsViewPinSetupHowto* instance);
View* desktop_settings_view_pin_setup_howto_get_view(DesktopSettingsViewPinSetupHowto* instance);
