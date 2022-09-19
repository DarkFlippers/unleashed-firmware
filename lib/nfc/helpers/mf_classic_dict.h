#pragma once

#include <stdbool.h>
#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/toolbox/stream/file_stream.h>
#include <lib/toolbox/stream/buffered_file_stream.h>

typedef enum {
    MfClassicDictTypeUser,
    MfClassicDictTypeFlipper,
} MfClassicDictType;

typedef struct MfClassicDict MfClassicDict;

bool mf_classic_dict_check_presence(MfClassicDictType dict_type);

MfClassicDict* mf_classic_dict_alloc(MfClassicDictType dict_type);

void mf_classic_dict_free(MfClassicDict* dict);

uint32_t mf_classic_dict_get_total_keys(MfClassicDict* dict);

bool mf_classic_dict_rewind(MfClassicDict* dict);

bool mf_classic_dict_is_key_present(MfClassicDict* dict, uint8_t* key);

bool mf_classic_dict_is_key_present_str(MfClassicDict* dict, string_t key);

bool mf_classic_dict_get_next_key(MfClassicDict* dict, uint64_t* key);

bool mf_classic_dict_get_next_key_str(MfClassicDict* dict, string_t key);

bool mf_classic_dict_get_key_at_index(MfClassicDict* dict, uint64_t* key, uint32_t target);

bool mf_classic_dict_get_key_at_index_str(MfClassicDict* dict, string_t key, uint32_t target);

bool mf_classic_dict_add_key(MfClassicDict* dict, uint8_t* key);

bool mf_classic_dict_add_key_str(MfClassicDict* dict, string_t key);

bool mf_classic_dict_find_index(MfClassicDict* dict, uint8_t* key, uint32_t* target);

bool mf_classic_dict_find_index_str(MfClassicDict* dict, string_t key, uint32_t* target);

bool mf_classic_dict_delete_index(MfClassicDict* dict, uint32_t target);
