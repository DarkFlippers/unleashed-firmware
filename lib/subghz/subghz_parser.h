#pragma once

#include "protocols/subghz_protocol_common.h"

typedef void (*SubGhzProtocolTextCallback)(string_t text, void* context);
typedef void (*SubGhzProtocolCommonCallbackDump)(SubGhzProtocolCommon* parser, void* context);

typedef struct SubGhzParser SubGhzParser;

/** Allocate SubGhzParser
 * 
 * @return SubGhzParser* 
 */
SubGhzParser* subghz_parser_alloc();

/** Free SubGhzParser
 * 
 * @param instance 
 */
void subghz_parser_free(SubGhzParser* instance);

/** Get protocol by name
 * 
 * @param instance - SubGhzParser instance
 * @param name - name protocol
 * @param SubGhzProtocolCommon
 */
SubGhzProtocolCommon* subghz_parser_get_by_name(SubGhzParser* instance, const char* name);

/** Outputting data text from all parsers
 * 
 * @param instance - SubGhzParser instance
 * @param callback - SubGhzProtocolTextCallback callback
 * @param context
 */
void subghz_parser_enable_dump_text(
    SubGhzParser* instance,
    SubGhzProtocolTextCallback callback,
    void* context);

/** Outputting data SubGhzParser from all parsers
 * 
 * @param instance - SubGhzParser instance
 * @param callback - SubGhzProtocolTextCallback callback
 * @param context
 */
void subghz_parser_enable_dump(
    SubGhzParser* instance,
    SubGhzProtocolCommonCallbackDump callback,
    void* context);

/** File name rainbow table Nice Flor-S
 * 
 * @param instance - SubGhzParser instance
 * @param file_name - "path/file_name"
 */
void subghz_parser_load_nice_flor_s_file(SubGhzParser* instance, const char* file_name);

/** File name rainbow table Came Atomo
 * 
 * @param instance - SubGhzParser instance
 * @param file_name - "path/file_name"
 */
void subghz_parser_load_came_atomo_file(SubGhzParser* instance, const char* file_name);

/** File upload manufacture keys
 * 
 * @param instance - SubGhzParser instance
 * @param file_name - "path/file_name"
 */
void subghz_parser_load_keeloq_file(SubGhzParser* instance, const char* file_name);

/** Restarting all parsers
 * 
 * @param instance - SubGhzParser instance
 */
void subghz_parser_reset(SubGhzParser* instance);

void subghz_parser_raw_parse(SubGhzParser* instance, bool level, uint32_t duration);

/** Loading data into all parsers
 * 
 * @param instance - SubGhzParser instance
 * @param level - true is high, false if low
 * @param duration - level duration in microseconds
 */
void subghz_parser_parse(SubGhzParser* instance, bool level, uint32_t duration);
