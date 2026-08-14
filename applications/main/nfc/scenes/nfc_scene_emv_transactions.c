#include "../helpers/protocol_support/emv/emv_extra_scenes.h"
#include "../helpers/protocol_support/nfc_protocol_support.h"

void nfc_scene_emv_transactions_on_enter(void* context) {
    nfc_protocol_support_extra_on_enter(NfcProtocolEmv, EmvExtraSceneTransactions, context);
}

bool nfc_scene_emv_transactions_on_event(void* context, SceneManagerEvent event) {
    return nfc_protocol_support_extra_on_event(
        NfcProtocolEmv, EmvExtraSceneTransactions, context, event);
}

void nfc_scene_emv_transactions_on_exit(void* context) {
    nfc_protocol_support_extra_on_exit(NfcProtocolEmv, EmvExtraSceneTransactions, context);
}
