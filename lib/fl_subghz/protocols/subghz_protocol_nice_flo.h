#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNiceFlo SubGhzProtocolNiceFlo;

/** Allocate SubGhzProtocolNiceFlo
 * 
 * @return SubGhzProtocolNiceFlo* 
 */
SubGhzProtocolNiceFlo* subghz_protocol_nice_flo_alloc();

/** Free SubGhzProtocolNiceFlo
 * 
 * @param instance 
 */
void subghz_protocol_nice_flo_free(SubGhzProtocolNiceFlo* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolNiceFlo instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_nice_flo_send_key(SubGhzProtocolNiceFlo* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolNiceFlo instance
 * @param data - LevelPair data
 */
void subghz_protocol_nice_flo_parse(SubGhzProtocolNiceFlo* instance, LevelPair data);
