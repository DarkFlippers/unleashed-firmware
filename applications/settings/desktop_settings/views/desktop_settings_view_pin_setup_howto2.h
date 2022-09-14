#pragma once

#include <gui/view.h>

typedef struct DesktopSettingsViewPinSetupHowto2 DesktopSettingsViewPinSetupHowto2;

typedef void (*DesktopSettingsViewPinSetupHowto2Callback)(void*);

DesktopSettingsViewPinSetupHowto2* desktop_settings_view_pin_setup_howto2_alloc();
void desktop_settings_view_pin_setup_howto2_free(DesktopSettingsViewPinSetupHowto2* instance);
View* desktop_settings_view_pin_setup_howto2_get_view(DesktopSettingsViewPinSetupHowto2* instance);
void desktop_settings_view_pin_setup_howto2_set_context(
    DesktopSettingsViewPinSetupHowto2* instance,
    void* context);
void desktop_settings_view_pin_setup_howto2_set_cancel_callback(
    DesktopSettingsViewPinSetupHowto2* instance,
    DesktopSettingsViewPinSetupHowto2Callback callback);
void desktop_settings_view_pin_setup_howto2_set_ok_callback(
    DesktopSettingsViewPinSetupHowto2* instance,
    DesktopSettingsViewPinSetupHowto2Callback callback);
