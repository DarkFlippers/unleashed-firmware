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

// SL3 geometry derived from the card size (0 for unknown).
uint16_t mf_plus_get_block_count(MfPlusSize size);

uint8_t mf_plus_get_sector_count(MfPlusSize size);

// Per-sector geometry (4K layout: sectors 0-31 have 4 blocks, 32-39 have 16).
uint16_t mf_plus_sector_get_first_block(uint8_t sector);

uint8_t mf_plus_sector_get_block_count(uint8_t sector);

// Known-tracking accessors. The set_* variants copy the payload and flip the mask bit together
// (bytes and "known" bit can never desync) and bounds-check their index.
bool mf_plus_is_block_read(const MfPlusData* data, uint16_t block_num);

void mf_plus_set_block_read(MfPlusData* data, uint16_t block_num, const MfPlusBlock* block);

bool mf_plus_is_key_found(const MfPlusData* data, uint8_t sector, MfPlusKeyType key_type);

void mf_plus_set_key_found(
    MfPlusData* data,
    uint8_t sector,
    MfPlusKeyType key_type,
    const MfPlusKey* key);

bool mf_plus_is_admin_key_found(const MfPlusData* data, MfPlusAdminKeyType type);

void mf_plus_set_admin_key_found(MfPlusData* data, MfPlusAdminKeyType type, const MfPlusKey* key);

bool mf_plus_is_config_block_read(const MfPlusData* data, uint8_t index);

void mf_plus_set_config_block_read(MfPlusData* data, uint8_t index, const MfPlusBlock* block);

// Serialize / parse the SL3 payload (data format version, signature, blocks, keys, config).
// Save writes the payload after the identity metadata; load tolerates legacy dumps that have
// no "Data format version" key (returns true, leaving the SL3 fields zeroed).
bool mf_plus_sl3_data_save(const MfPlusData* data, FlipperFormat* ff);

bool mf_plus_sl3_data_load(MfPlusData* data, FlipperFormat* ff);
