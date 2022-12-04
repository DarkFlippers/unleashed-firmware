//----------------------------------------------------------------------------- ----------------------------------------
// Biblio: [standing on the shoulders of giants]
//    https://bootlin.com/labs/doc/nunchuk.pdf
//    https://www.hackster.io/infusion/using-a-wii-nunchuk-with-arduino-597254#toc-i2c-protocol-9
//    https://web.archive.org/web/20220000000000*/https://www.hackster.io/infusion/using-a-wii-nunchuk-with-arduino-597254
//    https://github.com/madhephaestus/WiiChuck/blob/master/src/Accessory.cpp#L14
//    https://wiibrew.org/wiki/Wiimote/Extension_Controllers
//    https://www.best-microcontroller-projects.com/i2c-tutorial.html
//
// WiiMote Extension Controller:
//    Bus Address : 0x52
//    Register autoincrements after each (byte is) read
//       0x00..0x05 ( 6 bytes) ... [r] Controller Data
//       0x20..0x2F (16 bytes) ... [r] Calibration Data
//       0x30..0x3F (16 bytes) ... [r] (A copy of the) Calibration Data
//       0x40..0x4F (16 bytes) ... [w] Encryption key(s)
//       0xFA..0xFF ( 6 bytes) ... [r] Perhipheral ID

//----------------------------------------------------------------------------- ----------------------------------------
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>

#include "i2c_workaround.h" //! temporary workaround for a bug in furi i2c [see header]

#include "wii_anal.h"
#include "wii_i2c.h"
#include "wii_ec.h"

#include "bc_logging.h"

//----------------------------------------------------------------------------- ----------------------------------------
// Wii Extension Controller i2c Bus address
static const uint8_t ec_i2cAddr = 0x52;

// Initialise for UNencrypted comms
static const uint8_t regInit1 = 0xF0;
static const uint8_t regInit2 = 0xFB;
static const uint8_t cmdInit1[] = {regInit1, 0x55};
static const uint8_t cmdInit2[] = {regInit2, 0x00};

// Initialise for ENcrypted comms
static const uint8_t regInitEnc = 0x40;
static const uint8_t cmdInitEnc[] = {regInitEnc, 0x00};

// Crypto key (PSK),      base register : {0x40..0x4F}[2][8]
static const uint8_t regEnc = 0x40; // ENC_LEN

// Controller State data, base register : {0x00..0x05}[6]
static const uint8_t regJoy = 0x00; // JOY_LEN

// Calibration data,      base register : {0x20..0x2F}[16]
static const uint8_t regCal = 0x20; // CAL_LEN

// Controller ID,         base register : {0xFA..0xFF}[6]
static const uint8_t regPid = 0xFA; // PID_LEN

//+============================================================================ ========================================
// Hexdump a buffer to the logfile
//
#if LOG_LEVEL >= 4 // INFO

static void dump(const uint8_t* buf, const unsigned int len, const char* id) {
    // snprintf() would be useful!
    char s[128] = {0};
    char* p = NULL;

    strcpy(s, id);
    p = s + strlen(s);
    *p++ = ':';
    *p++ = ' ';
    *p++ = '{';

    for(unsigned int i = 0; i < len; i++) {
        uint8_t hi = (buf[i] & 0xF0) >> 4;
        uint8_t lo = (buf[i] & 0x0F);

        hi = hi + ((hi > 9) ? ('A' - 10) : '0');
        lo = lo + ((lo > 9) ? ('A' - 10) : '0');

        *p++ = (char)hi;
        *p++ = (char)lo;
        *p++ = ',';
    }
    *p = '\0';
    *--p = '}';
    INFO(s);
}

#else
#define dump(...)
#endif

//+============================================================================ ========================================
//
//! -W-A-R-N-I-N-G-  :  THIS ENCRYPTION CODE SHOULD NEVER BE REQUIRED ... AS SUCH, I'VE NEVER TESTED IT
//
static void decrypt(uint8_t* buf, const uint8_t* encKey, const uint8_t reg, unsigned int len) {
#if 1 // Use standard algorithm
    // decrypted_byte = (encrypted_byte XOR encKey[1][address%8]) + encKey[2][address%8]
    for(uint8_t* p = buf; p < buf + len; p++)
        *p = (*p ^ encKey[(reg + (p - buf)) % 8]) + encKey[8 + ((reg + (p - buf)) % 8)];

#else //! This is (I think) a shortcut for an all-zero key [not tested]
    (void)encKey;
    (void)reg;
    for(uint8_t* p = buf; p < buf + len; p++) *p = (*p ^ 0x17) + 0x17;
#endif
}

//+============================================================================ ========================================
// Read the Extension Controller state
// ...and decode it in to something sane
//
// Returns: {0:OK, >0:Error}
//
int ecRead(wiiEC_t* pec) {
    ENTER;
    int rv = 0; // assume success

    if(!pec->init) {
        WARN("%s : device not initialised", __func__);
        rv = 1;
        goto bail;
    }

    if(!furi_hal_i2c_is_device_ready(i2cBus, i2cAddr, i2cTimeout)) {
        INFO("%s : device disconnected", __func__);
        pec->init = false;
        rv = 2;
        goto bail;
    }

    if(!furi_hal_i2c_trxd(
           i2cBus, i2cAddr, &regJoy, 1, pec->joy, JOY_LEN, i2cTimeout, i2cReadWait)) {
        ERROR("%s : trxd fail", __func__);
        rv = 3;
        goto bail;
    }

    if(pec->encrypt) decrypt(pec->joy, pec->encKey, regJoy, JOY_LEN);

    // Decode the readings (according to Controller type)
    ecDecode(pec);

bail:
    LEAVE;
    return rv;
}

//+============================================================================ ========================================
// Initialise an Extension Controller
//
//! To disable encryption, pass a NULL encryption key  <-- this is currently ALWAYS the case
//
bool ecInit(wiiEC_t* pec, const uint8_t* encKey) {
    ENTER;

    bool rv = false; // assume failure

#if 0 //! i2c workaround
	//! I think this is done during OS startup - long before the plugin starts
	furi_hal_i2c_init();
#endif

#if 0 //! i2c workaround
	// May become relevant when the i2c issues are resolved
	// Take control of the i2c bus [which returns void !?]
	// --> firmware/targets/f7/furi_hal/furi_hal_i2c.c
	furi_hal_i2c_acquire(i2cBus);
#endif

    pec->init = false; // assume failure

    // === See if the device is alive ===
    if(!furi_hal_i2c_is_device_ready(i2cBus, i2cAddr, i2cTimeout)) {
        TRACE("%s : waiting for device", __func__);
        goto bail;
    }
    INFO("%s : device connected", __func__);

    // === Initialise the device ===
    pec->init = false; // This goes true AFTER the (optional) controller-specific init code

    // === Start the Extension Controller ===
    if(encKey) { //! start in encrypted mode

        //! todo - should this happen here, or AFTER we've got the ID ?

    } else {
        if(!furi_hal_i2c_tx(i2cBus, i2cAddr, cmdInit1, sizeof(cmdInit1), i2cTimeout)) {
            ERROR("%s : init fail (dec1)", __func__);
            goto bail;
        }
        TRACE("%s : init OK1", __func__);

        if(!furi_hal_i2c_tx(i2cBus, i2cAddr, cmdInit2, sizeof(cmdInit2), i2cTimeout)) {
            ERROR("%s : init fail (dec2)", __func__);
            goto bail;
        }
        TRACE("%s : init OK2", __func__);
    }

    // === Retrieve the Extension Controller ID ===
    if(!furi_hal_i2c_trx(i2cBus, i2cAddr, &regPid, 1, pec->pid, PID_LEN, i2cTimeout)) {
        ERROR("%s : T(R)x fail (pid)", __func__);
        goto bail;
    }
    if(pec->encrypt) decrypt(pec->joy, pec->encKey, regJoy, JOY_LEN);
    dump(pec->pid, PID_LEN, "pid"); // debug INFO

    // Find the StringID in the lookup table
    for(pec->pidx = PID_FIRST; pec->pidx < PID_ERROR; pec->pidx++)
        if(memcmp(pec->pid, ecId[pec->pidx].id, PID_LEN) == 0) break;
    if(pec->pidx == PID_ERROR) pec->pidx = PID_UNKNOWN;
    pec->sid = ecId[pec->pidx].name;
    INFO("sid: %s", pec->sid);

    // === (optionally) Enable encryption ===
    if(!encKey) {
        pec->encrypt = false;

    } else { // Controller WILL encrypt ALL tranmissions
        //! this encryption code fails - should it be done earlier?
        //! as it is probably never of any use, I'm kinda loathed to spend time on it
        //! https://github.com/madhephaestus/WiiChuck/blob/master/src/Accessory.cpp#L138
        uint8_t encTx[1 + ENC_LEN] = {0};
        uint8_t* ep = encTx;

        pec->encrypt = true;

        // ** Start the Controller in ENcrytped mode
        if(!furi_hal_i2c_tx(i2cBus, i2cAddr, cmdInitEnc, sizeof(cmdInitEnc), i2cTimeout)) {
            ERROR("%s : init fail (enc)", __func__);
            goto bail;
        }

        // Copy the (symmetric) encryption key to the controller state table
        if(pec->encKey != encKey) memcpy(pec->encKey, encKey, ENC_LEN);

        // Build the encryption key packet
        *ep++ = regEnc;
        memcpy(ep, pec->encKey, ENC_LEN);

        // ** Send encryption key (PSK)
        if(!furi_hal_i2c_tx(i2cBus, i2cAddr, encTx, (1 + ENC_LEN), i2cTimeout)) {
            ERROR("%s : key fail", __func__);
            goto bail;
        }

        TRACE("%s : init OK (enc)", __func__);
    }

    // === Some devices [eg. Drawsome/uDraw] require additional init code ===
    if(ecId[pec->init].init && (ecId[pec->init].init(pec) == false)) goto bail;
    pec->init = true;

    // === Read calibration data ===
    if(!furi_hal_i2c_trx(i2cBus, i2cAddr, &regCal, 1, pec->calF, CAL_LEN, i2cTimeout)) {
        ERROR("%s : trx fail (cal)", __func__);
        goto bail;
    }
    if(pec->encrypt) decrypt(pec->joy, pec->encKey, regJoy, JOY_LEN);
    dump(pec->calF, CAL_LEN, "cal");

    ecCalibrate(pec, CAL_RESET | CAL_FACTORY); // Load factory default calibration

    // === Initialise decode buffers ===
    pec->decN = 0; // read in to decode[1] (yes, N=0 -> read in to dec[1])
    switch(ecRead(pec)) {
    case 0: // read OK
        memcpy(&pec->dec[0], &pec->dec[1], sizeof(pec->dec[0]));
        dump(pec->joy, JOY_LEN, "joy");
        break;

    default: // bug: unknown
    case 1: // bug: not initialised - should never happen
        ERROR("%s : read bug", __func__);
        break;

    case 2: // device gone
    case 3: // read fail
        // Logging done by ecRead()
        pec->init = false;
        goto bail;
    }

    rv = true; // yay :)

bail:
#if 0 //! i2c workaround
	furi_hal_i2c_release(i2cBus);
#endif

    LEAVE;
    return rv;
}
