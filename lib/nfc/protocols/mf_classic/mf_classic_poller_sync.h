#pragma once

#include "mf_classic.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

MfClassicError mf_classic_poller_sync_collect_nt(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKeyType key_type,
    MfClassicNt* nt);

MfClassicError mf_classic_poller_sync_auth(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicAuthContext* data);

MfClassicError mf_classic_poller_sync_read_block(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicBlock* data);

MfClassicError mf_classic_poller_sync_write_block(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    MfClassicBlock* data);

MfClassicError mf_classic_poller_sync_read_value(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    int32_t* value);

MfClassicError mf_classic_poller_sync_change_value(
    Nfc* nfc,
    uint8_t block_num,
    MfClassicKey* key,
    MfClassicKeyType key_type,
    int32_t data,
    int32_t* new_value);

MfClassicError mf_classic_poller_sync_detect_type(Nfc* nfc, MfClassicType* type);

MfClassicError
    mf_classic_poller_sync_read(Nfc* nfc, const MfClassicDeviceKeys* keys, MfClassicData* data);

#ifdef __cplusplus
}
#endif
