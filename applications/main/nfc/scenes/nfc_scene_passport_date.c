#include "../nfc_i.h"
#include "m-string.h"
#include <gui/modules/validators.h>

#define TAG "PassportDate"

#define DATE_LENGTH 6

//TODO: use types in .h file? also in nfc_scene_passport_bac.c
#define NFC_PASSPORT_DATE_BIRTH 0
#define NFC_PASSPORT_DATE_EXPIRY 1

void nfc_scene_passport_date_text_input_callback(void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventTextInputDone);
}

void nfc_scene_passport_date_on_enter(void* context) {
    Nfc* nfc = context;

    MrtdDate date_value;

    uint32_t date_type = scene_manager_get_scene_state(nfc->scene_manager, NfcScenePassportDate);

    //TODO: numbers only
    TextInput* text_input = nfc->text_input;

    switch(date_type) {
    case NFC_PASSPORT_DATE_BIRTH:
        text_input_set_header_text(text_input, "Birth Date");
        date_value = nfc->dev->dev_data.mrtd_data.auth.birth_date;
        break;
    case NFC_PASSPORT_DATE_EXPIRY:
        text_input_set_header_text(text_input, "Expiry Date");
        date_value = nfc->dev->dev_data.mrtd_data.auth.expiry_date;
        break;
    }

    bool date_empty = false;
    if(date_value.year == 0 || date_value.month == 0 || date_value.day == 0 ||
       date_value.year > 100 || date_value.month > 13 || date_value.day > 31) {
        nfc_text_store_set(nfc, "YYMMDD");
        date_empty = true;
    } else {
        char temp_str[10];
        snprintf(temp_str, 10, "%02u%02u%02u", date_value.year, date_value.month, date_value.day);

        memcpy(nfc->text_store, temp_str, DATE_LENGTH);
        nfc->text_store[DATE_LENGTH] = '\x00';
    }

    text_input_set_result_callback(
        text_input,
        nfc_scene_passport_date_text_input_callback,
        nfc,
        nfc->text_store,
        DATE_LENGTH + 1, // incl. '\x00'
        date_empty); // Use as template

    //TODO: add validator for valid date (YYMMDD)

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextInput);
}

bool nfc_scene_passport_date_save(Nfc* nfc) {
    int year;
    int month;
    int day;
    int ret = sscanf(nfc->text_store, "%02d%02d%02d", &year, &month, &day);

    if(ret != 3) {
        FURI_LOG_E(TAG, "Invalid date entered (YYMMDD): %s", nfc->text_store);
        return false;
    }

    MrtdDate date_value;
    date_value.year = year;
    date_value.month = month;
    date_value.day = day;

    uint32_t date_type = scene_manager_get_scene_state(nfc->scene_manager, NfcScenePassportDate);

    //TODO: use types in .h file? also in nfc_scene_passport_bac.c
    switch(date_type) {
    case NFC_PASSPORT_DATE_BIRTH:
        nfc->dev->dev_data.mrtd_data.auth.birth_date = date_value;
        break;
    case NFC_PASSPORT_DATE_EXPIRY:
        nfc->dev->dev_data.mrtd_data.auth.expiry_date = date_value;
        break;
    }

    return true;
}

bool nfc_scene_passport_date_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventTextInputDone) {
            nfc_scene_passport_date_save(nfc);
            //TODO: handle invalid date (returned false)

            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc->scene_manager, NfcScenePassportAuth);
        }
    }
    return consumed;
}

void nfc_scene_passport_date_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    // TODO: clear validator

    text_input_reset(nfc->text_input);
}
