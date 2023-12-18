#pragma once

#include <furi/core/string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MfUserDict MfUserDict;

MfUserDict* mf_user_dict_alloc(size_t max_keys_to_load);

void mf_user_dict_free(MfUserDict* instance);

size_t mf_user_dict_get_keys_cnt(MfUserDict* instance);

void mf_user_dict_get_key_str(MfUserDict* instance, uint32_t index, FuriString* str);

bool mf_user_dict_delete_key(MfUserDict* instance, uint32_t index);

#ifdef __cplusplus
}
#endif
