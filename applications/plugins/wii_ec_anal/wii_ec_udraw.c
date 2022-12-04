//! udraw support is NOT written - this is just notes about the init function
#include <stdint.h>
#include <furi.h> // Core API

#include "wii_anal.h"
#include "wii_ec.h"
#include "bc_logging.h"

#include "i2c_workaround.h" //! temporary workaround for a bug in furi i2c [see header]

// ** If you want to see what this source code looks like with all the MACROs expanded
// **   grep -v '#include  ' wii_ec_udraw.c | gcc -E -o /dev/stdout -xc -
#include "wii_ec_macros.h"

//+============================================================================ ========================================
// https://github.com/madhephaestus/WiiChuck/blob/master/src/Drawsome.cpp#L3
// Gratuitously stolen ... never tested (don't own one) - just bought one on ebay <shrug>
//    although it seems like the UK version is a "uDraw" and MIGHT contain a different chipset :/
//
//   read  6 bytes starting from 0x20
//   read  6 bytes starting from 0x28
//   read  6 bytes starting from 0x30
//   read  6 bytes starting from 0x38
//   read  6 bytes starting from 0x00 (#1)
//   read  6 bytes starting from 0x00 (#2)
//   write 1 byte  [0x01]   to   0xFB
//   read  6 bytes starting from 0x00 (#3)
//   read  6 bytes starting from 0x00 (#4)
//
bool udraw_init(wiiEC_t* const pec) {
    ENTER;
    bool rv = true;

    (void)pec;
    /*
//! this is the Drawsome code, NOT the uDraw code !!
	static const uint8_t  reg[9] = {0x20, 0x28, 0x30, 0x38, 0x00, 0x00, 0xFB, 0x00, 0x00};  // 0..8
	const uint8_t*        p      = reg;
	uint8_t               buf[6] = {0};

	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 0
	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 1
	furi_delay_ms(100);

	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 2
	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 3
	furi_delay_ms(100);

	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 4
	furi_delay_ms(100);

	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 5
	furi_delay_ms(100);

	buf[0] = *p++;
	buf[1] = 0x01;
	if (!furi_hal_i2c_tx(bus,addr, buf,2, timeout))  goto fail ;  // 6

	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 7
	furi_delay_ms(100);

	if (!furi_hal_i2c_trxd(bus,addr, p++,1, buf,sizeof(buf), timeout,300))  goto fail ;  // 8
	furi_delay_ms(100);

	TRACE("%s : OK #%d", __func__, (p-reg));
	goto done;

fail:
	ERROR("%s : fail #%d", __func__, (p -reg) -1);
	rv = false;

done:
*/
    LEAVE;
    return rv;
}

//+============================================================================ ========================================
bool udraw_key(const eventMsg_t* const msg, state_t* const state) {
    (void)state;
    bool run = true;

    switch(msg->input.type) {
    case InputTypeShort: //# <!  After InputTypeRelease within INPUT_LONG_PRESS interval
        switch(msg->input.key) {
        case InputKeyUp: //# <U [ SHORT-UP ]
        case InputKeyDown: //# <D [ SHORT-DOWN ]
        case InputKeyLeft: //# <L [ SHORT-LEFT ]
        case InputKeyRight: //# <R [ SHORT-RIGHT ]
        case InputKeyOk: //# <O [ SHORT-OK ]
        case InputKeyBack: //# <B [ SHORT-BACK ]
        default:
            break; //# <?
        }
        break;
    case InputTypeLong: //# >!  After INPUT_LONG_PRESS interval, asynch to InputTypeRelease
        switch(msg->input.key) {
        case InputKeyUp: //# >U [ LONG-UP ]
        case InputKeyDown: //# >D [ LONG-DOWN ]
        case InputKeyLeft: //# >L [ LONG-LEFT ]
        case InputKeyRight: //# >R [ LONG-RIGHT ]
        case InputKeyOk: //# >O [ LONG-OK ]
        case InputKeyBack: //# >B [ LONG-BACK ]
        default:
            break; //# >?
        }
        break;
    case InputTypePress: //# +!  After debounce
        switch(msg->input.key) {
        case InputKeyUp: //# +U [ SHORT-UP ]
        case InputKeyDown: //# +D [ SHORT-DOWN ]
        case InputKeyLeft: //# +L [ SHORT-LEFT ]
        case InputKeyRight: //# +R [ SHORT-RIGHT ]
        case InputKeyOk: //# +O [ SHORT-OK ]
        case InputKeyBack: //# +B [ SHORT-BACK ]
        default:
            break; //# +?
        }
        break;
    case InputTypeRepeat: //# *!  With INPUT_REPEATE_PRESS period after InputTypeLong event
        switch(msg->input.key) {
        case InputKeyUp: //# *U [ REPEAT-UP ]
        case InputKeyDown: //# *D [ REPEAT-DOWN ]
        case InputKeyLeft: //# *L [ REPEAT-LEFT ]
        case InputKeyRight: //# *R [ REPEAT-RIGHT ]
        case InputKeyOk: //# *O [ REPEAT-OK ]
        case InputKeyBack: //# *B [ REPEAT-BACK ]
        default:
            break; //# *?
        }
        break;
    case InputTypeRelease: //# -!  After debounce
        switch(msg->input.key) {
        case InputKeyUp: //# -U [ RELEASE-UP ]
        case InputKeyDown: //# -D [ RELEASE-DOWN ]
        case InputKeyLeft: //# -L [ RELEASE-LEFT ]
        case InputKeyRight: //# -R [ RELEASE-RIGHT ]
        case InputKeyOk: //# -O [ RELEASE-OK ]
        case InputKeyBack: //# -B [ RELEASE-BACK ]
        default:
            break; //# -?
        }
        break;
    default:
        return true;
    }

    return run;
}
