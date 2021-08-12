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

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolGateTX instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_gate_tx_send_key(SubGhzProtocolGateTX* instance, uint64_t key, uint8_t bit, uint8_t repeat);

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

void subghz_protocol_gate_tx_to_save_str(SubGhzProtocolGateTX* instance, string_t output);
bool subghz_protocol_gate_tx_to_load_protocol(FileWorker* file_worker, SubGhzProtocolGateTX* instance);
