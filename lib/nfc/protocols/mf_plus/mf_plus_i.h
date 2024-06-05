#pragma once

#include "mf_plus.h"

#define MF_PLUS_FFF_PICC_PREFIX "PICC"

MfPlusError mf_plus_get_type_from_version(
    const Iso14443_4aData* iso14443_4a_data,
    MfPlusData* mf_plus_data);

MfPlusError mf_plus_get_type_from_iso4(const Iso14443_4aData* iso4_data, MfPlusData* mf_plus_data);

MfPlusError mf_plus_version_parse(MfPlusVersion* data, const BitBuffer* buf);

bool mf_plus_version_load(MfPlusVersion* data, FlipperFormat* ff);

bool mf_plus_security_level_load(MfPlusSecurityLevel* data, FlipperFormat* ff);

bool mf_plus_type_load(MfPlusType* data, FlipperFormat* ff);

bool mf_plus_size_load(MfPlusSize* data, FlipperFormat* ff);

bool mf_plus_version_save(const MfPlusVersion* data, FlipperFormat* ff);

bool mf_plus_security_level_save(const MfPlusSecurityLevel* data, FlipperFormat* ff);

bool mf_plus_type_save(const MfPlusType* data, FlipperFormat* ff);

bool mf_plus_size_save(const MfPlusSize* data, FlipperFormat* ff);
