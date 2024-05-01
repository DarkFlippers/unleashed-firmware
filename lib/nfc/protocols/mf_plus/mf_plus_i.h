#pragma once

#include "mf_plus.h"

#define MF_PLUS_FFF_PICC_PREFIX "PICC"

bool mf_plus_version_parse(MfPlusVersion* data, const BitBuffer* buf);

bool mf_plus_security_level_parse(MfPlusSecurityLevel* data, const BitBuffer* buf);

bool mf_plus_type_parse(MfPlusType* data, const BitBuffer* buf);

bool mf_plus_size_parse(MfPlusSize* data, const BitBuffer* buf);

bool mf_plus_version_load(MfPlusVersion* data, FlipperFormat* ff);

bool mf_plus_security_level_load(MfPlusSecurityLevel* data, FlipperFormat* ff);

bool mf_plus_type_load(MfPlusType* data, FlipperFormat* ff);

bool mf_plus_size_load(MfPlusSize* data, FlipperFormat* ff);

bool mf_plus_version_save(const MfPlusVersion* data, FlipperFormat* ff);

bool mf_plus_security_level_save(const MfPlusSecurityLevel* data, FlipperFormat* ff);

bool mf_plus_type_save(const MfPlusType* data, FlipperFormat* ff);

bool mf_plus_size_save(const MfPlusSize* data, FlipperFormat* ff);
