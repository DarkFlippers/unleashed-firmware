#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   RCA protocol description
*   https://www.sbprojects.net/knowledge/ir/rca.php
****************************************************************************************************
*     Preamble   Preamble      Pulse Distance/Width          Pause       Preamble   Preamble
*       mark      space            Modulation             up to period    repeat     repeat
*                                                                          mark       space
*
*        4000      4000               24 bit              ...8000          4000       4000
*     __________          _ _ _ _  _  _  _ _ _  _  _ _ _                ___________           
* ____          __________ _ _ _ __ __ __ _ _ __ __ _ _ ________________           ___________
*
***************************************************************************************************/

void* infrared_decoder_rca_alloc(void);
void infrared_decoder_rca_reset(void* decoder);
void infrared_decoder_rca_free(void* decoder);
InfraredMessage* infrared_decoder_rca_check_ready(void* decoder);
InfraredMessage* infrared_decoder_rca_decode(void* decoder, bool level, uint32_t duration);

void* infrared_encoder_rca_alloc(void);
InfraredStatus infrared_encoder_rca_encode(void* encoder_ptr, uint32_t* duration, bool* level);
void infrared_encoder_rca_reset(void* encoder_ptr, const InfraredMessage* message);
void infrared_encoder_rca_free(void* encoder_ptr);

const InfraredProtocolVariant* infrared_protocol_rca_get_variant(InfraredProtocol protocol);
