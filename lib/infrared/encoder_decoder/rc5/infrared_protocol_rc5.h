#pragma once

#include "../infrared_i.h"

/***************************************************************************************************
*   RC5 protocol description
*   https://www.mikrocontroller.net/articles/IRMP_-_english#RC5_.2B_RC5X
****************************************************************************************************
*                                       Manchester/biphase
*                                           Modulation
*
*                              888/1776 - bit (x2 for toggle bit)
*
*                           __  ____    __  __  __  __  __  __  __  __
*                         __  __    ____  __  __  __  __  __  __  __  _
*                         | 1 | 1 | 0 |      ...      |      ...      |
*                           s  si   T   address (MSB)   command (MSB)
*
*    Note: manchester starts from space timing, so it have to be handled properly
*    s - start bit (always 1)
*    si - RC5: start bit (always 1), RC5X - 7-th bit of address (in our case always 0)
*    T - toggle bit, change it's value every button press
*    address - 5 bit
*    command - 6/7 bit
***************************************************************************************************/

void* infrared_decoder_rc5_alloc(void);
void infrared_decoder_rc5_reset(void* decoder);
void infrared_decoder_rc5_free(void* decoder);
InfraredMessage* infrared_decoder_rc5_check_ready(void* ctx);
InfraredMessage* infrared_decoder_rc5_decode(void* decoder, bool level, uint32_t duration);

void* infrared_encoder_rc5_alloc(void);
void infrared_encoder_rc5_reset(void* encoder_ptr, const InfraredMessage* message);
void infrared_encoder_rc5_free(void* decoder);
InfraredStatus infrared_encoder_rc5_encode(void* encoder_ptr, uint32_t* duration, bool* polarity);

const InfraredProtocolVariant* infrared_protocol_rc5_get_variant(InfraredProtocol protocol);
