#include "nfc_cli_dump_mf_classic.h"
#include <nfc/protocols/mf_classic/mf_classic_poller.h>

#define TAG "MFC"

NfcCommand nfc_cli_dump_poller_callback_mf_classic(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCliDumpContext* instance = context;
    const MfClassicPollerEvent* mfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfClassic, nfc_poller_get_data(instance->poller));
        size_t uid_len = 0;
        const uint8_t* uid = nfc_device_get_uid(instance->nfc_device, &uid_len);
        if(mf_classic_key_cache_load(instance->mfc_key_cache, uid, uid_len)) {
            FURI_LOG_D(TAG, "Key cache found");
            mfc_event->data->poller_mode.mode = MfClassicPollerModeRead;
        } else {
            FURI_LOG_D(TAG, "Key cache not found");
            instance->result = NfcCliDumpErrorFailedToRead;
            command = NfcCommandStop;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestReadSector) {
        uint8_t sector_num = 0;
        MfClassicKey key = {};
        MfClassicKeyType key_type = MfClassicKeyTypeA;
        if(mf_classic_key_cache_get_next_key(
               instance->mfc_key_cache, &sector_num, &key, &key_type)) {
            mfc_event->data->read_sector_request_data.sector_num = sector_num;
            mfc_event->data->read_sector_request_data.key = key;
            mfc_event->data->read_sector_request_data.key_type = key_type;
            mfc_event->data->read_sector_request_data.key_provided = true;
        } else {
            mfc_event->data->read_sector_request_data.key_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfClassic, nfc_poller_get_data(instance->poller));
        instance->result = NfcCliDumpErrorNone;
        command = NfcCommandStop;
    } else if(mfc_event->type == MfClassicPollerEventTypeFail) {
        instance->result = NfcCliDumpErrorFailedToRead;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_semaphore_release(instance->sem_done);
    }

    return command;
}
