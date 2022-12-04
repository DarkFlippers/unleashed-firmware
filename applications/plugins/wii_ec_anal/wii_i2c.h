#ifndef WII_I2C_H_
#define WII_I2C_H_

#include <stdint.h>

//#include  "wii_ec.h"

//----------------------------------------------------------------------------- ----------------------------------------
// i2c bus details
//
// https://www.best-microcontroller-projects.com/i2c-tutorial.html
// https://web.archive.org/web/20220000000000*/https://www.best-microcontroller-projects.com/i2c-tutorial.html
// https://training.ti.com/introduction-i2c-reserved-addresses
//
// After the (special) START "bit"...
//   the first 8bits (byte) of i2c data are the 7bit i2c Address,
//   FOLLOWED by 1bit to signify a READ or WRITE {0=write, 1=read}
// The data is transmitted BIG-Endian, IE. MSb first [human readable]
// So the address actually lives in the TOP (MSb's) of the first "byte", (with bit0 being used as the read/write flag)
//
// The read() and write() functions on the FZ will set the LSb appropriately,
//   BUT they do NOT shift the address left to make room for it!
// So the address you give to read/write() MUST be given as (7bitAddress << 1)
//
// When we read:  After we send the read command, we wait for i2cReadWait uS before reading the data
//

// firmware/targets/f7/furi_hal/furi_hal_i2c_types.h
#define i2cBus (&furi_hal_i2c_handle_external) // FZ external i2c bus
#define i2cAddr (ec_i2cAddr << 1)
#define i2cTimeout (3) // in mS
#define i2cReadWait (300) //! 300uS: how low can we take this?

//----------------------------------------------------------------------------- ----------------------------------------
// public functions
//
typedef struct wiiEC wiiEC_t;

bool ecInit(wiiEC_t* const pec, const uint8_t* encKey);
int ecRead(wiiEC_t* const pec);

#endif //WII_I2C_H_
