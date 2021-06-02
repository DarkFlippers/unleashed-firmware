#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "irda.h"


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
#define IRDA_NEC_REPEAT_PAUSE           40000
#define IRDA_NEC_REPEAT_PAUSE_MAX       150000
#define IRDA_NEC_REPEAT_MARK            9000
#define IRDA_NEC_REPEAT_SPACE           2250
#define IRDA_NEC_CARRIER_FREQUENCY      38000
#define IRDA_NEC_DUTY_CYCLE             0.33
#define IRDA_NEC_PREAMBLE_TOLERANCE     0.07    // percents
#define IRDA_NEC_BIT_TOLERANCE          120     // us

void* irda_decoder_nec_alloc(void);
void* irda_decoder_necext_alloc(void);
void irda_encoder_nec_encode(uint32_t address, uint32_t command, bool repeat);
void irda_encoder_necext_encode(uint32_t address, uint32_t command, bool repeat);
void irda_decoder_nec_reset(void* decoder);
void irda_decoder_nec_free(void* decoder);
IrdaMessage* irda_decoder_nec_decode(void* decoder, bool level, uint32_t duration);


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
#define IRDA_SAMSUNG_REPEAT_PAUSE           47000
#define IRDA_SAMSUNG_REPEAT_PAUSE_MAX       150000
#define IRDA_SAMSUNG_REPEAT_MARK            4500
#define IRDA_SAMSUNG_REPEAT_SPACE           4500
#define IRDA_SAMSUNG_CARRIER_FREQUENCY      38000
#define IRDA_SAMSUNG_DUTY_CYCLE             0.33
#define IRDA_SAMSUNG_PREAMBLE_TOLERANCE     0.07    // percents
#define IRDA_SAMSUNG_BIT_TOLERANCE          120     // us

void* irda_decoder_samsung32_alloc(void);
void irda_encoder_samsung32_encode(uint32_t address, uint32_t command, bool repeat);
void irda_decoder_samsung32_reset(void* decoder);
void irda_decoder_samsung32_free(void* decoder);
IrdaMessage* irda_decoder_samsung32_decode(void* decoder, bool level, uint32_t duration);

