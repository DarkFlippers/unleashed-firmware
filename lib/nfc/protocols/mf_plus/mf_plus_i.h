#pragma once

#include "mf_plus.h"

#include <nfc/helpers/nxp_native_command.h>

#define MF_PLUS_FFF_PICC_PREFIX "PICC"

// probed_security_level carries the result of the poller's active SAK-0x20 probe (SL0 or SL3), or
// MfPlusSecurityLevelUnknown when no probe applies/succeeded; it only influences the SAK-0x20 path.
MfPlusError mf_plus_get_type_from_version(
    const Iso14443_4aData* iso14443_4a_data,
    MfPlusSecurityLevel probed_security_level,
    MfPlusData* mf_plus_data);

MfPlusError mf_plus_get_type_from_iso4(
    const Iso14443_4aData* iso4_data,
    MfPlusSecurityLevel probed_security_level,
    MfPlusData* mf_plus_data);

MfPlusError mf_plus_version_parse(MfPlusVersion* data, const BitBuffer* buf);

bool mf_plus_version_load(MfPlusVersion* data, FlipperFormat* ff);

bool mf_plus_security_level_load(MfPlusSecurityLevel* data, FlipperFormat* ff);

bool mf_plus_type_load(MfPlusType* data, FlipperFormat* ff);

bool mf_plus_size_load(MfPlusSize* data, FlipperFormat* ff);

bool mf_plus_version_save(const MfPlusVersion* data, FlipperFormat* ff);

bool mf_plus_security_level_save(const MfPlusSecurityLevel* data, FlipperFormat* ff);

bool mf_plus_type_save(const MfPlusType* data, FlipperFormat* ff);

bool mf_plus_size_save(const MfPlusSize* data, FlipperFormat* ff);

// Set accessors: copy the payload and flip the mask bit together (bytes and "known" bit can never
// desync) and bounds-check their index. The matching is_* readers are public.
void mf_plus_set_block_read(MfPlusData* data, uint16_t block_num, const MfPlusBlock* block);

void mf_plus_set_key_found(
    MfPlusData* data,
    uint8_t sector,
    MfPlusKeyType key_type,
    const MfPlusKey* key);

void mf_plus_set_admin_key_found(MfPlusData* data, MfPlusAdminKeyType type, const MfPlusKey* key);

void mf_plus_set_config_block_read(MfPlusData* data, uint8_t index, const MfPlusBlock* block);

// Map an admin key <-> its 0x90xx auth address (single source: mf_plus_admin_key_addresses).
uint16_t mf_plus_get_admin_key_address(MfPlusAdminKeyType type);

bool mf_plus_admin_key_type_from_address(uint16_t address, MfPlusAdminKeyType* type);

// Serialize / parse the SL3 payload (data format version, signature, blocks, keys, config).
// Save writes the payload after the identity metadata; load tolerates legacy dumps that have
// no "Data format version" key (returns true, leaving the SL3 fields zeroed).
bool mf_plus_sl3_data_save(const MfPlusData* data, FlipperFormat* ff);

bool mf_plus_sl3_data_load(MfPlusData* data, FlipperFormat* ff);
