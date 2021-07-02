#pragma once

#include "subghz_protocol_common.h"

typedef void (*SubGhzProtocolTextCallback)(string_t text, void* context);

typedef struct SubGhzProtocol SubGhzProtocol;

/** Allocate SubGhzProtocol
 * 
 * @return SubGhzProtocol* 
 */
SubGhzProtocol* subghz_protocol_alloc();

/** Free SubGhzProtocol
 * 
 * @param instance 
 */
void subghz_protocol_free(SubGhzProtocol* instance);

/** Outputting data from all parsers
 * 
 * @param instance - SubGhzProtocol instance
 * @param callback - SubGhzProtocolTextCallback callback
 * @param context
 */
void subghz_protocol_enable_dump(
    SubGhzProtocol* instance,
    SubGhzProtocolTextCallback callback,
    void* context);

/** File name rainbow table Nice Flor-S
 * 
 * @param instance - SubGhzProtocol instance
 * @param file_name - "path/file_name"
 */
void subghz_protocol_load_nice_flor_s_file(SubGhzProtocol* instance, const char* file_name);

/** File upload manufacture keys
 * 
 * @param instance - SubGhzProtocol instance
 * @param file_name - "path/file_name"
 */
void subghz_protocol_load_keeloq_file(SubGhzProtocol* instance, const char* file_name);

/** Restarting all parsers
 * 
 * @param instance - SubGhzProtocol instance
 */
void subghz_protocol_reset(SubGhzProtocol* instance);

/** Loading data into all parsers
 * 
 * @param instance - SubGhzProtocol instance
 * @param data - LevelPair data
 */
void subghz_protocol_parse(SubGhzProtocol* instance, LevelPair data);
