#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   Kaseikyo protocol description
*   https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_Kaseikyo.hpp
****************************************************************************************************
*     Preamble   Preamble      Pulse Distance/Width          Pause       Preamble   Preamble
*       mark      space            Modulation             up to period    repeat     repeat
*                                                                          mark       space
*
*        3360      1665               48 bit              ...130000        3456       1728
*     __________          _ _ _ _  _  _  _ _ _  _  _ _ _                ___________
* ____          __________ _ _ _ __ __ __ _ _ __ __ _ _ ________________           ___________
*
***************************************************************************************************/

void* infrared_decoder_kaseikyo_alloc(void);
void infrared_decoder_kaseikyo_reset(void* decoder);
void infrared_decoder_kaseikyo_free(void* decoder);
InfraredMessage* infrared_decoder_kaseikyo_check_ready(void* decoder);
InfraredMessage* infrared_decoder_kaseikyo_decode(void* decoder, bool level, uint32_t duration);

void* infrared_encoder_kaseikyo_alloc(void);
InfraredStatus
    infrared_encoder_kaseikyo_encode(void* encoder_ptr, uint32_t* duration, bool* level);
void infrared_encoder_kaseikyo_reset(void* encoder_ptr, const InfraredMessage* message);
void infrared_encoder_kaseikyo_free(void* encoder_ptr);

const InfraredProtocolVariant* infrared_protocol_kaseikyo_get_variant(InfraredProtocol protocol);
