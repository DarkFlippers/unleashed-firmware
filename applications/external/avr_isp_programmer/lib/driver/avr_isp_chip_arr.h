#pragma once

#include <furi_hal.h>

#define F_AVR8L 1 // TPI programming, ATtiny(4|5|9|10|20|40|102|104)
#define F_AVR8 2 // ISP programming with SPI, "classic" AVRs
#define F_XMEGA 4 // PDI programming, ATxmega family
#define F_AVR8X 8 // UPDI programming, newer 8-bit MCUs

struct AvrIspChipArr { // Value of -1 typically means unknown
    const char* name; // Name of part
    uint16_t mcuid; // ID of MCU in 0..2039
    uint8_t avrarch; // F_AVR8L, F_AVR8, F_XMEGA or F_AVR8X
    uint8_t sigs[3]; // Signature bytes
    int32_t flashoffset; // Flash offset
    int32_t flashsize; // Flash size
    int16_t pagesize; // Flash page size
    int8_t nboots; // Number of supported boot sectors
    int16_t bootsize; // Size of (smallest) boot sector
    int32_t eepromoffset; // EEPROM offset
    int32_t eepromsize; // EEPROM size
    int32_t eeprompagesize; // EEPROM page size
    int32_t sramstart; // SRAM offset
    int32_t sramsize; // SRAM size
    int8_t nfuses; // Number of fuse bytes
    int8_t nlocks; // Number of lock bytes
    uint8_t ninterrupts; // Number of vectors in interrupt vector table
};

typedef struct AvrIspChipArr AvrIspChipArr;

extern const AvrIspChipArr avr_isp_chip_arr[];
extern const size_t avr_isp_chip_arr_size;