#pragma once

#include <gui/view.h>
#include "../helpers/nfc_rfid_detector_types.h"
#include "../helpers/nfc_rfid_detector_event.h"

typedef struct NfcRfidDetectorFieldPresence NfcRfidDetectorFieldPresence;

void nfc_rfid_detector_view_field_presence_update(
    NfcRfidDetectorFieldPresence* instance,
    bool nfc_field,
    bool rfid_field,
    uint32_t rfid_frequency);

NfcRfidDetectorFieldPresence* nfc_rfid_detector_view_field_presence_alloc();

void nfc_rfid_detector_view_field_presence_free(NfcRfidDetectorFieldPresence* instance);

View* nfc_rfid_detector_view_field_presence_get_view(NfcRfidDetectorFieldPresence* instance);
