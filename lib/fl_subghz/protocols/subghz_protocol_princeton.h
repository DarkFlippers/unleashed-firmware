#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolPrinceton SubGhzProtocolPrinceton;

SubGhzProtocolPrinceton* subghz_protocol_princeton_alloc();

void subghz_protocol_princeton_free(SubGhzProtocolPrinceton* instance);

void subghz_protocol_princeton_send_key(SubGhzProtocolPrinceton* instance, uint64_t key, uint8_t bit, uint8_t repeat);

void subghz_protocol_princeton_parse(SubGhzProtocolPrinceton* instance, LevelPair data);

void subghz_protocol_princeton_to_str(SubGhzProtocolPrinceton* instance, string_t output);
