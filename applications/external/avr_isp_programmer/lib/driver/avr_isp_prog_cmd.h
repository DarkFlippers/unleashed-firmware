#pragma once

// http://ww1.microchip.com/downloads/en/appnotes/atmel-0943-in-system-programming_applicationnote_avr910.pdf
// AVR ISP Definitions
#define AVR_ISP_HWVER 0X02
#define AVR_ISP_SWMAJ 0X01
#define AVR_ISP_SWMIN 0X12
#define AVP_ISP_SERIAL_CONNECT_TYPE 0X53
#define AVP_ISP_CONNECT_TYPE 0x93
#define AVR_ISP_RESP_0 0X00

#define AVR_ISP_SET_PMODE 0xAC, 0x53, 0x00, 0x00
#define AVR_ISP_READ_VENDOR 0x30, 0x00, 0x00, 0x00
#define AVR_ISP_READ_PART_FAMILY 0x30, 0x00, 0x01, 0x00
#define AVR_ISP_READ_PART_NUMBER 0x30, 0x00, 0x02, 0x00
#define AVR_ISP_ERASE_CHIP \
    0xAC, 0x80, 0x00, 0x00 //Erase Chip, Wait N ms, Release RESET to end the erase.
//The only way to end a Chip Erase cycle is by temporarily releasing the Reset line

#define AVR_ISP_EXTENDED_ADDR(data) 0x4D, 0x00, data, 0x00
#define AVR_ISP_WRITE_FLASH_LO(add, data) 0x40, (add >> 8) & 0xFF, add & 0xFF, data
#define AVR_ISP_WRITE_FLASH_HI(add, data) 0x48, (add >> 8) & 0xFF, add & 0xFF, data
#define AVR_ISP_READ_FLASH_LO(add) 0x20, (add >> 8) & 0xFF, add & 0xFF, 0x00
#define AVR_ISP_READ_FLASH_HI(add) 0x28, (add >> 8) & 0xFF, add & 0xFF, 0x00

#define AVR_ISP_WRITE_EEPROM(add, data) \
    0xC0, (add >> 8) & 0xFF, add & 0xFF, data //Send cmd, Wait N ms
#define AVR_ISP_READ_EEPROM(add) 0xA0, (add >> 8) & 0xFF, add & 0xFF, 0xFF

#define AVR_ISP_COMMIT(add) \
    0x4C, (add >> 8) & 0xFF, add & 0xFF, 0x00 //Send cmd, polling read last addr page

#define AVR_ISP_OSCCAL(add) 0x38, 0x00, add, 0x00

#define AVR_ISP_WRITE_LOCK_BYTE(data) 0xAC, 0xE0, 0x00, data //Send cmd, Wait N ms
#define AVR_ISP_READ_LOCK_BYTE 0x58, 0x00, 0x00, 0x00
#define AVR_ISP_WRITE_FUSE_LOW(data) 0xAC, 0xA0, 0x00, data //Send cmd, Wait N ms
#define AVR_ISP_READ_FUSE_LOW 0x50, 0x00, 0x00, 0x00
#define AVR_ISP_WRITE_FUSE_HIGH(data) 0xAC, 0xA8, 0x00, data //Send cmd, Wait N ms
#define AVR_ISP_READ_FUSE_HIGH 0x58, 0x08, 0x00, 0x00
#define AVR_ISP_WRITE_FUSE_EXTENDED(data) 0xAC, 0xA4, 0x00, data //Send cmd, Wait N ms (~write)
#define AVR_ISP_READ_FUSE_EXTENDED 0x50, 0x08, 0x00, 0x00

#define AVR_ISP_EECHUNK 0x20

// https://www.microchip.com/content/dam/mchp/documents/OTH/ApplicationNotes/ApplicationNotes/doc2525.pdf
// STK Definitions
#define STK_OK 0x10
#define STK_FAILED 0x11
#define STK_UNKNOWN 0x12
#define STK_INSYNC 0x14
#define STK_NOSYNC 0x15
#define CRC_EOP 0x20

#define STK_GET_SYNC 0x30
#define STK_GET_SIGN_ON 0x31
#define STK_SET_PARAMETER 0x40
#define STK_GET_PARAMETER 0x41
#define STK_SET_DEVICE 0x42
#define STK_SET_DEVICE_EXT 0x45
#define STK_ENTER_PROGMODE 0x50
#define STK_LEAVE_PROGMODE 0x51
#define STK_CHIP_ERASE 0x52
#define STK_CHECK_AUTOINC 0x53
#define STK_LOAD_ADDRESS 0x55
#define STK_UNIVERSAL 0x56
#define STK_UNIVERSAL_MULTI 0x57
#define STK_PROG_FLASH 0x60
#define STK_PROG_DATA 0x61
#define STK_PROG_FUSE 0x62
#define STK_PROG_FUSE_EXT 0x65
#define STK_PROG_LOCK 0x63
#define STK_PROG_PAGE 0x64
#define STK_READ_FLASH 0x70
#define STK_READ_DATA 0x71
#define STK_READ_FUSE 0x72
#define STK_READ_LOCK 0x73
#define STK_READ_PAGE 0x74
#define STK_READ_SIGN 0x75
#define STK_READ_OSCCAL 0x76
#define STK_READ_FUSE_EXT 0x77
#define STK_READ_OSCCAL_EXT 0x78
#define STK_HW_VER 0x80
#define STK_SW_MAJOR 0x81
#define STK_SW_MINOR 0x82
#define STK_LEDS 0x83
#define STK_VTARGET 0x84
#define STK_VADJUST 0x85
#define STK_OSC_PSCALE 0x86
#define STK_OSC_CMATCH 0x87
#define STK_SCK_DURATION 0x89
#define STK_BUFSIZEL 0x90
#define STK_BUFSIZEH 0x91
#define STK_STK500_TOPCARD_DETECT 0x98

#define STK_SET_EEPROM_TYPE 0X45
#define STK_SET_FLASH_TYPE 0X46
