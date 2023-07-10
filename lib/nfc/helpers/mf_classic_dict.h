#pragma once

#include <stdbool.h>
#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/toolbox/stream/file_stream.h>
#include <lib/toolbox/stream/buffered_file_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MfClassicDictTypeUser,
    MfClassicDictTypeSystem,
    MfClassicDictTypeUnitTest,
} MfClassicDictType;

typedef struct MfClassicDict MfClassicDict;

bool mf_classic_dict_check_presence(MfClassicDictType dict_type);

/** Allocate MfClassicDict instance
 *
 * @param[in]  dict_type  The dictionary type
 *
 * @return     MfClassicDict instance
 */
MfClassicDict* mf_classic_dict_alloc(MfClassicDictType dict_type);

/** Free MfClassicDict instance
 *
 * @param      dict  MfClassicDict instance
 */
void mf_classic_dict_free(MfClassicDict* dict);

/** Get total keys count
 *
 * @param      dict  MfClassicDict instance
 *
 * @return     total keys count
 */
uint32_t mf_classic_dict_get_total_keys(MfClassicDict* dict);

/** Rewind to the beginning
 *
 * @param      dict  MfClassicDict instance
 *
 * @return     true on success
 */
bool mf_classic_dict_rewind(MfClassicDict* dict);

bool mf_classic_dict_is_key_present(MfClassicDict* dict, uint8_t* key);

bool mf_classic_dict_is_key_present_str(MfClassicDict* dict, FuriString* key);

bool mf_classic_dict_get_next_key(MfClassicDict* dict, uint64_t* key);

bool mf_classic_dict_get_next_key_str(MfClassicDict* dict, FuriString* key);

/** Get key at target offset as uint64_t
 *
 * @param      dict    MfClassicDict instance
 * @param[out] key     Pointer to the uint64_t key
 * @param[in]  target  Target offset from current position
 *
 * @return     true on success
 */
bool mf_classic_dict_get_key_at_index(MfClassicDict* dict, uint64_t* key, uint32_t target);

/** Get key at target offset as FuriString*
 *
 * @param      dict    MfClassicDict instance
 * @param[out] key     Found key destination buffer
 * @param[in]  target  Target offset from current position
 *
 * @return     true on success
 */
bool mf_classic_dict_get_key_at_index_str(MfClassicDict* dict, FuriString* key, uint32_t target);

bool mf_classic_dict_add_key(MfClassicDict* dict, uint8_t* key);

/** Add string representation of the key
 *
 * @param      dict  MfClassicDict instance
 * @param[in]  key   String representation of the key
 *
 * @return     true on success
 */
bool mf_classic_dict_add_key_str(MfClassicDict* dict, FuriString* key);

bool mf_classic_dict_find_index(MfClassicDict* dict, uint8_t* key, uint32_t* target);

bool mf_classic_dict_find_index_str(MfClassicDict* dict, FuriString* key, uint32_t* target);

/** Delete key at target offset
 *
 * @param      dict    MfClassicDict instance
 * @param[in]  target  Target offset from current position
 *
 * @return     true on success
 */
bool mf_classic_dict_delete_index(MfClassicDict* dict, uint32_t target);

#ifdef __cplusplus
}
#endif