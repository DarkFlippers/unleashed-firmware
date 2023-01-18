#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   RC6 protocol description
*   https://www.mikrocontroller.net/articles/IRMP_-_english#RC6_.2B_RC6A
****************************************************************************************************
*      Preamble                       Manchester/biphase                       Silence
*     mark/space                          Modulation
*
*    2666     889        444/888 - bit (x2 for toggle bit)                       2666
*
*  ________         __    __  __  __    ____  __  __  __  __  __  __  __  __
* _        _________  ____  __  __  ____    __  __  __  __  __  __  __  __  _______________
*                   | 1 | 0 | 0 | 0 |   0   |      ...      |      ...      |             |
*                     s  m2  m1  m0     T     address (MSB)   command (MSB)
*
*    s - start bit (always 1)
*    m0-2 - mode (000 for RC6)
*    T - toggle bit, twice longer
*    address - 8 bit
*    command - 8 bit
***************************************************************************************************/

void* infrared_decoder_rc6_alloc(void);
void infrared_decoder_rc6_reset(void* decoder);
void infrared_decoder_rc6_free(void* decoder);
InfraredMessage* infrared_decoder_rc6_check_ready(void* ctx);
InfraredMessage* infrared_decoder_rc6_decode(void* decoder, bool level, uint32_t duration);

void* infrared_encoder_rc6_alloc(void);
void infrared_encoder_rc6_reset(void* encoder_ptr, const InfraredMessage* message);
void infrared_encoder_rc6_free(void* decoder);
InfraredStatus infrared_encoder_rc6_encode(void* encoder_ptr, uint32_t* duration, bool* polarity);

const InfraredProtocolVariant* infrared_protocol_rc6_get_variant(InfraredProtocol protocol);
