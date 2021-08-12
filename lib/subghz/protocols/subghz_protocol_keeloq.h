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

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolKeeloq instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_keeloq_send_key(SubGhzProtocolKeeloq* instance, uint64_t key, uint8_t bit, uint8_t repeat);

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

void subghz_protocol_keeloq_to_save_str(SubGhzProtocolKeeloq* instance, string_t output);
bool subghz_protocol_keeloq_to_load_protocol(FileWorker* file_worker, SubGhzProtocolKeeloq* instance);
