#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolKeeloq SubGhzProtocolKeeloq;

SubGhzProtocolKeeloq* subghz_protocol_keeloq_alloc();

void subghz_protocol_keeloq_free(SubGhzProtocolKeeloq* instance);

void subghz_protocol_keeloq_add_manafacture_key(SubGhzProtocolKeeloq* instance, const char* name, uint64_t key, uint16_t type);

void subghz_protocol_keeloq_send_key(SubGhzProtocolKeeloq* instance, uint64_t key, uint8_t bit, uint8_t repeat);

void subghz_protocol_keeloq_parse(SubGhzProtocolKeeloq* instance, LevelPair data);

void subghz_protocol_keeloq_to_str(SubGhzProtocolKeeloq* instance, string_t output);
