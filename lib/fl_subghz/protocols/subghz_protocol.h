#pragma once

#include "subghz_protocol_common.h"

typedef void (*SubGhzProtocolTextCallback)(string_t text, void* context);

typedef struct SubGhzProtocol SubGhzProtocol;

SubGhzProtocol* subghz_protocol_alloc();

void subghz_protocol_free(SubGhzProtocol* instance);

void subghz_protocol_enable_dump(SubGhzProtocol* instance, SubGhzProtocolTextCallback callback, void* context);

void subghz_protocol_load_keeloq_file(SubGhzProtocol* instance, const char* file_name);

void subghz_protocol_reset(SubGhzProtocol* instance);

void subghz_protocol_parse(SubGhzProtocol* instance, LevelPair data);
