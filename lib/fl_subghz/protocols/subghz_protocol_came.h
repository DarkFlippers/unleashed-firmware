#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolCame SubGhzProtocolCame;

SubGhzProtocolCame* subghz_protocol_came_alloc();

void subghz_protocol_came_free(SubGhzProtocolCame* instance);

void subghz_protocol_came_send_key(SubGhzProtocolCame* instance, uint64_t key, uint8_t bit, uint8_t repeat);

void subghz_protocol_came_parse(SubGhzProtocolCame* instance, LevelPair data);
