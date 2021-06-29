#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNiceFlo SubGhzProtocolNiceFlo;

SubGhzProtocolNiceFlo* subghz_protocol_nice_flo_alloc();

void subghz_protocol_nice_flo_free(SubGhzProtocolNiceFlo* instance);

void subghz_protocol_nice_flo_set_callback(SubGhzProtocolNiceFlo* instance, SubGhzProtocolCommonCallback callback, void* context);

void subghz_protocol_nice_flo_send_key(SubGhzProtocolNiceFlo* instance, uint64_t key, uint8_t bit, uint8_t repeat);

void subghz_protocol_nice_flo_parse(SubGhzProtocolNiceFlo* instance, LevelPair data);
