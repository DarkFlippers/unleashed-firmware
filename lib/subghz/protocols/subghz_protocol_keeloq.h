#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzKeystore SubGhzKeystore;

typedef struct SubGhzProtocolKeeloq SubGhzProtocolKeeloq;

/** Allocate SubGhzProtocolKeeloq
 * 
 * @return SubGhzProtocolKeeloq* 
 */
SubGhzProtocolKeeloq* subghz_protocol_keeloq_alloc(SubGhzKeystore* keystore);

/** Free SubGhzProtocolKeeloq
 * 
 * @param instance 
 */
void subghz_protocol_keeloq_free(SubGhzProtocolKeeloq* instance);

/** Find and get manufacture name
 * 
 * @param context - SubGhzProtocolKeeloq context
 * @return name - char* manufacture name
 */
const char* subghz_protocol_keeloq_find_and_get_manufacture_name(void* context);

/** Get manufacture name
 * 
 * @param context - SubGhzProtocolKeeloq context
 * @return name - char* manufacture name
 */
const char* subghz_protocol_keeloq_get_manufacture_name(void* context);

/** Set manufacture name
 * 
 * @param manufacture_name - manufacture name
 * @param context - SubGhzProtocolKeeloq context
 * @return bool
 */
bool subghz_protocol_keeloq_set_manufacture_name(void* context, const char* manufacture_name);

/** Get key keeloq
 * 
 * @param context - SubGhzProtocolKeeloq context
 * @return key
 */
uint64_t subghz_protocol_keeloq_gen_key(void* context);

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolKeeloq instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_keeloq_send_key(
    SubGhzProtocolKeeloq* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolKeeloq instance
 */
void subghz_protocol_keeloq_reset(SubGhzProtocolKeeloq* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolKeeloq instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_keeloq_parse(SubGhzProtocolKeeloq* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolKeeloq* instance
 * @param output   - output string
 */
void subghz_protocol_keeloq_to_str(SubGhzProtocolKeeloq* instance, string_t output);

/** Get a string to save the protocol
 * 
 * @param instance  - SubGhzProtocolKeeloq instance
 * @param output    - the resulting string
 */
void subghz_protocol_keeloq_to_save_str(SubGhzProtocolKeeloq* instance, string_t output);

/** Loading protocol from file
 * 
 * @param file_worker - FileWorker file_worker
 * @param instance - SubGhzProtocolKeeloq instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_keeloq_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolKeeloq* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolKeeloq instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_keeloq_to_load_protocol(SubGhzProtocolKeeloq* instance, void* context);
