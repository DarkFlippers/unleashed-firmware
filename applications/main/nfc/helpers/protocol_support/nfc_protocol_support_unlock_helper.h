#include "nfc/nfc_app_i.h"

typedef enum {
    NfcSceneReadMenuStateCardSearch,
    NfcSceneReadMenuStateCardFound,
} NfcSceneUnlockReadState;

void nfc_unlock_helper_setup_from_state(NfcApp* instance);
void nfc_unlock_helper_card_detected_handler(NfcApp* instance);
