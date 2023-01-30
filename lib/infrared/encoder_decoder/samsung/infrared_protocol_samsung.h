#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   SAMSUNG32 protocol description
*   https://www.mikrocontroller.net/articles/IRMP_-_english#SAMSUNG
****************************************************************************************************
*  Preamble   Preamble     Pulse Distance/Width        Pause       Preamble   Preamble  Bit1  Stop
*    mark      space           Modulation                           repeat     repeat          bit
*                                                                    mark       space
*
*     4500      4500        32 bit + stop bit       40000/100000     4500       4500
*  __________          _  _ _  _  _  _ _ _  _  _ _                ___________            _    _
* _          __________ __ _ __ __ __ _ _ __ __ _ ________________           ____________ ____ ___
*
***************************************************************************************************/

void* infrared_decoder_samsung32_alloc(void);
void infrared_decoder_samsung32_reset(void* decoder);
void infrared_decoder_samsung32_free(void* decoder);
InfraredMessage* infrared_decoder_samsung32_check_ready(void* ctx);
InfraredMessage* infrared_decoder_samsung32_decode(void* decoder, bool level, uint32_t duration);

InfraredStatus
    infrared_encoder_samsung32_encode(void* encoder_ptr, uint32_t* duration, bool* level);
void infrared_encoder_samsung32_reset(void* encoder_ptr, const InfraredMessage* message);
void* infrared_encoder_samsung32_alloc(void);
void infrared_encoder_samsung32_free(void* encoder_ptr);

const InfraredProtocolVariant* infrared_protocol_samsung32_get_variant(InfraredProtocol protocol);
