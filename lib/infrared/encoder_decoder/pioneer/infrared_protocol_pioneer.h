#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   Pioneer SR protocol description
*   http://www.adrian-kingston.com/IRFormatPioneer.htm
****************************************************************************************************
*      Preamble  Preamble     Pulse Width Modulation           Pause           Entirely repeat
*        mark     space                                     up to period          message..
*
*        8500      4250         33 bits (500, 1500)          ...26000          8500      4250
*     __________          _ _ _ _  _  _  _ _ _  _  _ _ _                    __________          _ _
* ____          __________ _ _ _ __ __ __ _ _ __ __ _ _ ____________________          __________ _
*
* In 33 bits of data there is:
*  - 8 bits address
*  - 8 bits address inverse
*  - 8 bits command
*  - 8 bits command inverse
*  - 1 stop bit
***************************************************************************************************/

void* infrared_decoder_pioneer_alloc(void);
void infrared_decoder_pioneer_reset(void* decoder);
InfraredMessage* infrared_decoder_pioneer_check_ready(void* decoder);
void infrared_decoder_pioneer_free(void* decoder);
InfraredMessage* infrared_decoder_pioneer_decode(void* decoder, bool level, uint32_t duration);

void* infrared_encoder_pioneer_alloc(void);
void infrared_encoder_pioneer_reset(void* encoder_ptr, const InfraredMessage* message);
void infrared_encoder_pioneer_free(void* decoder);
InfraredStatus
    infrared_encoder_pioneer_encode(void* encoder_ptr, uint32_t* duration, bool* polarity);

const InfraredProtocolVariant* infrared_protocol_pioneer_get_variant(InfraredProtocol protocol);
