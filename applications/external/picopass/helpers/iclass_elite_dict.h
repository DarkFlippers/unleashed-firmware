#pragma once

#include <stdbool.h>
#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/toolbox/stream/file_stream.h>
#include <lib/toolbox/stream/buffered_file_stream.h>

typedef enum {
    IclassEliteDictTypeUser,
    IclassEliteDictTypeFlipper,
    IclassStandardDictTypeFlipper,
} IclassEliteDictType;

typedef struct IclassEliteDict IclassEliteDict;

bool iclass_elite_dict_check_presence(IclassEliteDictType dict_type);

IclassEliteDict* iclass_elite_dict_alloc(IclassEliteDictType dict_type);

void iclass_elite_dict_free(IclassEliteDict* dict);

uint32_t iclass_elite_dict_get_total_keys(IclassEliteDict* dict);

bool iclass_elite_dict_get_next_key(IclassEliteDict* dict, uint8_t* key);

bool iclass_elite_dict_rewind(IclassEliteDict* dict);

bool iclass_elite_dict_add_key(IclassEliteDict* dict, uint8_t* key);
