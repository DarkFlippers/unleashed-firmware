/**
 * @file protocol_cyfral.h
 * 
 * Cyfral pulse format decoder
 */

#pragma once
#include <stdint.h>
#include "../../pulse_protocols/pulse_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ProtocolCyfral ProtocolCyfral;

/**
 * Allocate decoder
 * @return ProtocolCyfral* 
 */
ProtocolCyfral* protocol_cyfral_alloc();

/**
 * Deallocate decoder
 * @param cyfral 
 */
void protocol_cyfral_free(ProtocolCyfral* cyfral);

/**
 * Get protocol interface
 * @param cyfral 
 * @return PulseProtocol* 
 */
PulseProtocol* protocol_cyfral_get_protocol(ProtocolCyfral* cyfral);

#ifdef __cplusplus
}
#endif