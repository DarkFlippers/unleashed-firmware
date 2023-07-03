#include "nfc_rfid_detector_app_i.h"

#include <furi.h>

#define TAG "NfcRfidDetector"

void nfc_rfid_detector_app_field_presence_start(NfcRfidDetectorApp* app) {
    furi_assert(app);

    // start the field presence rfid detection
    furi_hal_rfid_field_detect_start();

    // start the field presence nfc detection
    furi_hal_nfc_exit_sleep();
    furi_hal_nfc_field_detect_start();
}

void nfc_rfid_detector_app_field_presence_stop(NfcRfidDetectorApp* app) {
    furi_assert(app);

    // stop the field presence rfid detection
    furi_hal_rfid_field_detect_stop();

    // stop the field presence nfc detection
    furi_hal_nfc_start_sleep();
}

bool nfc_rfid_detector_app_field_presence_is_nfc(NfcRfidDetectorApp* app) {
    furi_assert(app);

    // check if the field presence is nfc
    return furi_hal_nfc_field_is_present();
}

bool nfc_rfid_detector_app_field_presence_is_rfid(NfcRfidDetectorApp* app, uint32_t* frequency) {
    furi_assert(app);

    // check if the field presence is rfid
    return furi_hal_rfid_field_is_present(frequency);
}