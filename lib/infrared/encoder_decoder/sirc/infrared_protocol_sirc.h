#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   Sony SIRC protocol description
*   https://www.sbprojects.net/knowledge/ir/sirc.php
*   http://picprojects.org.uk/
****************************************************************************************************
*      Preamble  Preamble     Pulse Width Modulation           Pause             Entirely repeat
*        mark     space                                     up to period             message..
*
*        2400      600      12/15/20 bits (600,1200)         ...45000          2400      600
*     __________          _ _ _ _  _  _  _ _ _  _  _ _ _                    __________          _ _
* ____          __________ _ _ _ __ __ __ _ _ __ __ _ _ ____________________          __________ _
*                        |    command    |   address    |
*                 SIRC   |     7b LSB    |    5b LSB    |
*                 SIRC15 |     7b LSB    |    8b LSB    |
*                 SIRC20 |     7b LSB    |    13b LSB   |
*
* No way to determine either next message is repeat or not,
* so recognize only fact message received. Sony remotes always send at least 3 messages.
* Assume 8 last extended bits for SIRC20 are address bits.
***************************************************************************************************/

void* infrared_decoder_sirc_alloc(void);
void infrared_decoder_sirc_reset(void* decoder);
InfraredMessage* infrared_decoder_sirc_check_ready(void* decoder);
void infrared_decoder_sirc_free(void* decoder);
InfraredMessage* infrared_decoder_sirc_decode(void* decoder, bool level, uint32_t duration);

void* infrared_encoder_sirc_alloc(void);
void infrared_encoder_sirc_reset(void* encoder_ptr, const InfraredMessage* message);
void infrared_encoder_sirc_free(void* decoder);
InfraredStatus infrared_encoder_sirc_encode(void* encoder_ptr, uint32_t* duration, bool* polarity);

const InfraredProtocolVariant* infrared_protocol_sirc_get_variant(InfraredProtocol protocol);
