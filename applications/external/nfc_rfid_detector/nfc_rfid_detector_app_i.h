#pragma once

#include "helpers/nfc_rfid_detector_types.h"
#include "helpers/nfc_rfid_detector_event.h"

#include "scenes/nfc_rfid_detector_scene.h"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include "views/nfc_rfid_detector_view_field_presence.h"

typedef struct NfcRfidDetectorApp NfcRfidDetectorApp;

struct NfcRfidDetectorApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    Submenu* submenu;
    Widget* widget;
    NfcRfidDetectorFieldPresence* nfc_rfid_detector_field_presence;
};

void nfc_rfid_detector_app_field_presence_start(NfcRfidDetectorApp* app);
void nfc_rfid_detector_app_field_presence_stop(NfcRfidDetectorApp* app);
bool nfc_rfid_detector_app_field_presence_is_nfc(NfcRfidDetectorApp* app);
bool nfc_rfid_detector_app_field_presence_is_rfid(NfcRfidDetectorApp* app, uint32_t* frequency);