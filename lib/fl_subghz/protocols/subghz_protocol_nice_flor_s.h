#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNiceFlorS SubGhzProtocolNiceFlorS;

SubGhzProtocolNiceFlorS* subghz_protocol_nice_flor_s_alloc();

void subghz_protocol_nice_flor_s_free(SubGhzProtocolNiceFlorS* instance);

void subghz_protocol_nice_flor_s_set_callback(SubGhzProtocolNiceFlorS* instance, SubGhzProtocolCommonCallback callback, void* context);

void subghz_protocol_nice_flor_s_send_key(SubGhzProtocolNiceFlorS* instance, uint64_t key, uint8_t bit, uint8_t repeat);

void subghz_protocol_nice_flor_s_parse(SubGhzProtocolNiceFlorS* instance, LevelPair data);
