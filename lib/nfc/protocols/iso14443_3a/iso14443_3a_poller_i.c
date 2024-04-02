#include "iso14443_3a_poller_i.h"

#include <furi.h>

#include "nfc/helpers/iso14443_crc.h"

#define TAG "ISO14443_3A"

static Iso14443_3aError iso14443_3a_poller_process_error(NfcError error) {
    Iso14443_3aError ret = Iso14443_3aErrorNone;
    if(error == NfcErrorNone) {
        ret = Iso14443_3aErrorNone;
    } else if(error == NfcErrorTimeout) {
        ret = Iso14443_3aErrorTimeout;
    } else {
        ret = Iso14443_3aErrorNotPresent;
    }
    return ret;
}

static Iso14443_3aError iso14443_3a_poller_standard_frame_exchange(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);

    uint16_t tx_bytes = bit_buffer_get_size_bytes(tx_buffer);
    furi_assert(tx_bytes <= bit_buffer_get_capacity_bytes(instance->tx_buffer) - 2);

    bit_buffer_copy(instance->tx_buffer, tx_buffer);
    iso14443_crc_append(Iso14443CrcTypeA, instance->tx_buffer);
    Iso14443_3aError ret = Iso14443_3aErrorNone;

    do {
        NfcError error =
            nfc_poller_trx(instance->nfc, instance->tx_buffer, instance->rx_buffer, fwt);
        if(error != NfcErrorNone) {
            ret = iso14443_3a_poller_process_error(error);
            break;
        }

        bit_buffer_copy(rx_buffer, instance->rx_buffer);
        if(!iso14443_crc_check(Iso14443CrcTypeA, instance->rx_buffer)) {
            ret = Iso14443_3aErrorWrongCrc;
            break;
        }

        iso14443_crc_trim(rx_buffer);
    } while(false);

    return ret;
}

Iso14443_3aError iso14443_3a_poller_check_presence(Iso14443_3aPoller* instance) {
    furi_check(instance);
    furi_check(instance->nfc);

    NfcError error = NfcErrorNone;
    Iso14443_3aError ret = Iso14443_3aErrorNone;
    do {
        error = nfc_iso14443a_poller_trx_short_frame(
            instance->nfc,
            NfcIso14443aShortFrameSensReq,
            instance->rx_buffer,
            ISO14443_3A_FDT_LISTEN_FC);
        if(error != NfcErrorNone) {
            ret = iso14443_3a_poller_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(instance->col_res.sens_resp)) {
            ret = Iso14443_3aErrorCommunication;
            break;
        }
    } while(false);

    return ret;
}

Iso14443_3aError iso14443_3a_poller_halt(Iso14443_3aPoller* instance) {
    furi_check(instance);
    furi_check(instance->nfc);
    furi_check(instance->tx_buffer);

    uint8_t halt_cmd[2] = {0x50, 0x00};
    bit_buffer_copy_bytes(instance->tx_buffer, halt_cmd, sizeof(halt_cmd));

    iso14443_3a_poller_standard_frame_exchange(
        instance, instance->tx_buffer, instance->rx_buffer, ISO14443_3A_FDT_LISTEN_FC);

    instance->state = Iso14443_3aPollerStateIdle;
    return Iso14443_3aErrorNone;
}

Iso14443_3aError
    iso14443_3a_poller_activate(Iso14443_3aPoller* instance, Iso14443_3aData* iso14443_3a_data) {
    furi_check(instance);
    furi_check(instance->nfc);
    furi_check(instance->tx_buffer);
    furi_check(instance->rx_buffer);

    // Reset Iso14443_3a poller state
    memset(&instance->col_res, 0, sizeof(instance->col_res));
    memset(instance->data, 0, sizeof(Iso14443_3aData));
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Halt if necessary
    if(instance->state != Iso14443_3aPollerStateIdle) {
        iso14443_3a_poller_halt(instance);
        instance->state = Iso14443_3aPollerStateIdle;
    }

    NfcError error = NfcErrorNone;
    Iso14443_3aError ret = Iso14443_3aErrorNone;

    bool activated = false;
    do {
        error = nfc_iso14443a_poller_trx_short_frame(
            instance->nfc,
            NfcIso14443aShortFrameSensReq,
            instance->rx_buffer,
            ISO14443_3A_FDT_LISTEN_FC);
        if(error != NfcErrorNone) {
            ret = Iso14443_3aErrorNotPresent;
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(instance->col_res.sens_resp)) {
            FURI_LOG_W(TAG, "Wrong sens response size");
            ret = Iso14443_3aErrorCommunication;
            break;
        }
        bit_buffer_write_bytes(
            instance->rx_buffer,
            &instance->col_res.sens_resp,
            sizeof(instance->col_res.sens_resp));
        memcpy(
            instance->data->atqa,
            &instance->col_res.sens_resp,
            sizeof(instance->col_res.sens_resp));

        instance->state = Iso14443_3aPollerStateColResInProgress;
        instance->col_res.cascade_level = 0;
        instance->col_res.state = Iso14443_3aPollerColResStateStateNewCascade;

        while(instance->state == Iso14443_3aPollerStateColResInProgress) {
            if(instance->col_res.state == Iso14443_3aPollerColResStateStateNewCascade) {
                bit_buffer_set_size_bytes(instance->tx_buffer, 2);
                bit_buffer_set_byte(
                    instance->tx_buffer,
                    0,
                    ISO14443_3A_POLLER_SEL_CMD(instance->col_res.cascade_level));
                bit_buffer_set_byte(instance->tx_buffer, 1, ISO14443_3A_POLLER_SEL_PAR(2, 0));
                error = nfc_iso14443a_poller_trx_sdd_frame(
                    instance->nfc,
                    instance->tx_buffer,
                    instance->rx_buffer,
                    ISO14443_3A_FDT_LISTEN_FC);
                if(error != NfcErrorNone) {
                    FURI_LOG_E(TAG, "Sdd request failed: %d", error);
                    instance->state = Iso14443_3aPollerStateColResFailed;
                    ret = Iso14443_3aErrorColResFailed;
                    break;
                }
                if(bit_buffer_get_size_bytes(instance->rx_buffer) != 5) {
                    FURI_LOG_E(TAG, "Sdd response wrong length");
                    instance->state = Iso14443_3aPollerStateColResFailed;
                    ret = Iso14443_3aErrorColResFailed;
                    break;
                }
                bit_buffer_write_bytes(
                    instance->rx_buffer, &instance->col_res.sdd_resp, sizeof(Iso14443_3aSddResp));
                instance->col_res.state = Iso14443_3aPollerColResStateStateSelectCascade;
            } else if(instance->col_res.state == Iso14443_3aPollerColResStateStateSelectCascade) {
                instance->col_res.sel_req.sel_cmd =
                    ISO14443_3A_POLLER_SEL_CMD(instance->col_res.cascade_level);
                instance->col_res.sel_req.sel_par = ISO14443_3A_POLLER_SEL_PAR(7, 0);
                memcpy(
                    instance->col_res.sel_req.nfcid,
                    instance->col_res.sdd_resp.nfcid,
                    sizeof(instance->col_res.sdd_resp.nfcid));
                instance->col_res.sel_req.bcc = instance->col_res.sdd_resp.bss;
                bit_buffer_copy_bytes(
                    instance->tx_buffer,
                    (uint8_t*)&instance->col_res.sel_req,
                    sizeof(instance->col_res.sel_req));
                ret = iso14443_3a_poller_send_standard_frame(
                    instance, instance->tx_buffer, instance->rx_buffer, ISO14443_3A_FDT_LISTEN_FC);
                if(ret != Iso14443_3aErrorNone) {
                    FURI_LOG_E(TAG, "Sel request failed: %d", ret);
                    instance->state = Iso14443_3aPollerStateColResFailed;
                    ret = Iso14443_3aErrorColResFailed;
                    break;
                }
                if(bit_buffer_get_size_bytes(instance->rx_buffer) !=
                   sizeof(instance->col_res.sel_resp)) {
                    FURI_LOG_E(TAG, "Sel response wrong length");
                    instance->state = Iso14443_3aPollerStateColResFailed;
                    ret = Iso14443_3aErrorColResFailed;
                    break;
                }
                bit_buffer_write_bytes(
                    instance->rx_buffer,
                    &instance->col_res.sel_resp,
                    sizeof(instance->col_res.sel_resp));
                FURI_LOG_T(TAG, "Sel resp: %02X", instance->col_res.sel_resp.sak);
                if(instance->col_res.sel_req.nfcid[0] == ISO14443_3A_POLLER_SDD_CL) {
                    // Copy part of UID
                    memcpy(
                        &instance->data->uid[instance->data->uid_len],
                        &instance->col_res.sel_req.nfcid[1],
                        3);
                    instance->data->uid_len += 3;
                    instance->col_res.cascade_level++;
                    instance->col_res.state = Iso14443_3aPollerColResStateStateNewCascade;
                } else {
                    FURI_LOG_T(TAG, "Col resolution complete");
                    instance->data->sak = instance->col_res.sel_resp.sak;
                    memcpy(
                        &instance->data->uid[instance->data->uid_len],
                        &instance->col_res.sel_req.nfcid[0],
                        4);
                    instance->data->uid_len += 4;
                    instance->col_res.state = Iso14443_3aPollerColResStateStateSuccess;
                    instance->state = Iso14443_3aPollerStateActivated;
                }
            }
        }

        activated = (instance->state == Iso14443_3aPollerStateActivated);
    } while(false);

    if(activated && iso14443_3a_data) {
        *iso14443_3a_data = *instance->data;
    }

    return ret;
}

Iso14443_3aError iso14443_3a_poller_txrx_custom_parity(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    Iso14443_3aError ret = Iso14443_3aErrorNone;
    NfcError error =
        nfc_iso14443a_poller_trx_custom_parity(instance->nfc, tx_buffer, rx_buffer, fwt);
    if(error != NfcErrorNone) {
        ret = iso14443_3a_poller_process_error(error);
    }

    return ret;
}

Iso14443_3aError iso14443_3a_poller_txrx(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    Iso14443_3aError ret = Iso14443_3aErrorNone;
    NfcError error = nfc_poller_trx(instance->nfc, tx_buffer, rx_buffer, fwt);
    if(error != NfcErrorNone) {
        ret = iso14443_3a_poller_process_error(error);
    }

    return ret;
}

Iso14443_3aError iso14443_3a_poller_send_standard_frame(
    Iso14443_3aPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    Iso14443_3aError ret =
        iso14443_3a_poller_standard_frame_exchange(instance, tx_buffer, rx_buffer, fwt);

    return ret;
}
