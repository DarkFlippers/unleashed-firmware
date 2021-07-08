#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "irda.h"
#include "irda_common_i.h"

/***************************************************************************************************
*   NEC protocol description
*   https://radioparty.ru/manuals/encyclopedia/213-ircontrol?start=1
****************************************************************************************************
*     Preamble   Preamble      Pulse Distance/Width          Pause       Preamble   Preamble  Stop
*       mark      space            Modulation                             repeat     repeat    bit
*                                                                          mark       space
*
*        9000      4500         32 bit + stop bit         40000/100000     9000       2250
*     __________          _ _ _ _  _  _  _ _ _  _  _ _ _                ___________            _
* ____          __________ _ _ _ __ __ __ _ _ __ __ _ _ ________________           ____________ ___
*
***************************************************************************************************/

#define IRDA_NEC_PREAMBULE_MARK         9000
#define IRDA_NEC_PREAMBULE_SPACE        4500
#define IRDA_NEC_BIT1_MARK              560
#define IRDA_NEC_BIT1_SPACE             1600
#define IRDA_NEC_BIT0_MARK              560
#define IRDA_NEC_BIT0_SPACE             560
#define IRDA_NEC_REPEAT_PAUSE_MIN       30000
#define IRDA_NEC_REPEAT_PAUSE1          46000
#define IRDA_NEC_REPEAT_PAUSE2          97000
#define IRDA_NEC_SILENCE                IRDA_NEC_REPEAT_PAUSE2
#define IRDA_NEC_REPEAT_PAUSE_MAX       150000
#define IRDA_NEC_REPEAT_MARK            9000
#define IRDA_NEC_REPEAT_SPACE           2250
#define IRDA_NEC_PREAMBLE_TOLERANCE     0.07    // percents
#define IRDA_NEC_BIT_TOLERANCE          120     // us

void* irda_decoder_nec_alloc(void);
void irda_decoder_nec_reset(void* decoder);
void irda_decoder_nec_free(void* decoder);
IrdaMessage* irda_decoder_nec_decode(void* decoder, bool level, uint32_t duration);

void* irda_encoder_nec_alloc(void);
IrdaStatus irda_encoder_nec_encode(void* encoder_ptr, uint32_t* duration, bool* level);
void irda_encoder_nec_reset(void* encoder_ptr, const IrdaMessage* message);
void irda_encoder_nec_free(void* encoder_ptr);

void* irda_decoder_necext_alloc(void);
void* irda_encoder_necext_alloc(void);
void irda_encoder_necext_reset(void* encoder_ptr, const IrdaMessage* message);

bool irda_decoder_nec_interpret(IrdaCommonDecoder* decoder);
bool irda_decoder_necext_interpret(IrdaCommonDecoder* decoder);
IrdaStatus irda_decoder_nec_decode_repeat(IrdaCommonDecoder* decoder);
IrdaStatus irda_encoder_nec_encode_repeat(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level);

extern const IrdaCommonProtocolSpec protocol_necext;
extern const IrdaCommonProtocolSpec protocol_nec;


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

#define IRDA_SAMSUNG_PREAMBULE_MARK         4500
#define IRDA_SAMSUNG_PREAMBULE_SPACE        4500
#define IRDA_SAMSUNG_BIT1_MARK              550
#define IRDA_SAMSUNG_BIT1_SPACE             1650
#define IRDA_SAMSUNG_BIT0_MARK              550
#define IRDA_SAMSUNG_BIT0_SPACE             550
#define IRDA_SAMSUNG_REPEAT_PAUSE_MIN       30000
#define IRDA_SAMSUNG_REPEAT_PAUSE1          46000
#define IRDA_SAMSUNG_REPEAT_PAUSE2          97000
/* Samsung silence have to be greater than REPEAT MAX
 * otherwise there can be problems during unit tests parsing
 * of some data. Real tolerances we don't know, but in real life
 * silence time should be greater than max repeat time. This is
 * because of similar preambule timings for repeat and first messages. */
#define IRDA_SAMSUNG_SILENCE                145000
#define IRDA_SAMSUNG_REPEAT_PAUSE_MAX       140000
#define IRDA_SAMSUNG_REPEAT_MARK            4500
#define IRDA_SAMSUNG_REPEAT_SPACE           4500
#define IRDA_SAMSUNG_PREAMBLE_TOLERANCE     0.07    // percents
#define IRDA_SAMSUNG_BIT_TOLERANCE          120     // us

void* irda_decoder_samsung32_alloc(void);
void irda_decoder_samsung32_reset(void* decoder);
void irda_decoder_samsung32_free(void* decoder);
IrdaMessage* irda_decoder_samsung32_decode(void* decoder, bool level, uint32_t duration);

IrdaStatus irda_encoder_samsung32_encode(void* encoder_ptr, uint32_t* duration, bool* level);
void irda_encoder_samsung32_reset(void* encoder_ptr, const IrdaMessage* message);
void* irda_encoder_samsung32_alloc(void);
void irda_encoder_samsung32_free(void* encoder_ptr);

bool irda_decoder_samsung32_interpret(IrdaCommonDecoder* decoder);
IrdaStatus irda_decoder_samsung32_decode_repeat(IrdaCommonDecoder* decoder);
IrdaStatus irda_encoder_samsung32_encode_repeat(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level);

extern const IrdaCommonProtocolSpec protocol_samsung32;


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

#define IRDA_RC6_PREAMBULE_MARK             2666
#define IRDA_RC6_PREAMBULE_SPACE            889
#define IRDA_RC6_BIT                        444     // half of time-quant for 1 bit
#define IRDA_RC6_PREAMBLE_TOLERANCE         0.07    // percents
#define IRDA_RC6_BIT_TOLERANCE              120     // us
#define IRDA_RC6_SILENCE                    2700

void* irda_decoder_rc6_alloc(void);
void irda_decoder_rc6_reset(void* decoder);
void irda_decoder_rc6_free(void* decoder);
IrdaMessage* irda_decoder_rc6_decode(void* decoder, bool level, uint32_t duration);
void* irda_encoder_rc6_alloc(void);
void irda_encoder_rc6_reset(void* encoder_ptr, const IrdaMessage* message);
void irda_encoder_rc6_free(void* decoder);
IrdaStatus irda_encoder_rc6_encode(void* encoder_ptr, uint32_t* duration, bool* polarity);

bool irda_decoder_rc6_interpret(IrdaCommonDecoder* decoder);
IrdaStatus irda_decoder_rc6_decode_manchester(IrdaCommonDecoder* decoder);
IrdaStatus irda_encoder_rc6_encode_manchester(IrdaCommonEncoder* encoder_ptr, uint32_t* duration, bool* polarity);

extern const IrdaCommonProtocolSpec protocol_rc6;

