#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolGateTX SubGhzProtocolGateTX;

/** Allocate SubGhzProtocolGateTX
 * 
 * @return SubGhzProtocolGateTX* 
 */
SubGhzProtocolGateTX* subghz_protocol_gate_tx_alloc();

/** Free SubGhzProtocolGateTX
 * 
 * @param instance 
 */
void subghz_protocol_gate_tx_free(SubGhzProtocolGateTX* instance);

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolGateTX instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_gate_tx_send_key(
    SubGhzProtocolGateTX* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolGateTX instance
 */
void subghz_protocol_gate_tx_reset(SubGhzProtocolGateTX* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolGateTX instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_gate_tx_parse(SubGhzProtocolGateTX* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolFaacSLH* instance
 * @param output   - output string
 */
void subghz_protocol_gate_tx_to_str(SubGhzProtocolGateTX* instance, string_t output);

/** Get a string to save the protocol
 * 
 * @param instance  - SubGhzProtocolGateTX instance
 * @param output    - the resulting string
 */
void subghz_protocol_gate_tx_to_save_str(SubGhzProtocolGateTX* instance, string_t output);

/** Loading protocol from file
 * 
 * @param file_worker - FileWorker file_worker
 * @param instance - SubGhzProtocolGateTX instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_gate_tx_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolGateTX* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolGateTX instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_gate_tx_to_load_protocol(SubGhzProtocolGateTX* instance, void* context);
