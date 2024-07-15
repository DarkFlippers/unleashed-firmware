/*

  u8x8.h
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
  
  
  U8glib has several layers. Each layer is implemented with a callback function. 
  This callback function handels the messages for the layer.

  The topmost level is the display layer. It includes the following messages:
  
    U8X8_MSG_DISPLAY_SETUP_MEMORY			no communicaation with the display, setup memory ony
    U8X8_MSG_DISPLAY_INIT
    U8X8_MSG_DISPLAY_SET_FLIP_MODE
    U8X8_MSG_DISPLAY_SET_POWER_SAVE
    U8X8_MSG_DISPLAY_SET_CONTRAST
    U8X8_MSG_DISPLAY_DRAW_TILE

  A display driver may decided to breakdown these messages to a lower level interface or
  implement this functionality directly.
  

  One layer is the Command/Arg/Data interface. It can be used by the display layer
  to communicate with the display hardware.
  This layer only deals with data, commands and arguments. D/C line is unknown.
    U8X8_MSG_CAD_INIT
    U8X8_MSG_CAD_SET_I2C_ADR	(obsolete)
    U8X8_MSG_CAD_SET_DEVICE (obsolete)
    U8X8_MSG_CAD_START_TRANSFER
    U8X8_MSG_CAD_SEND_CMD
    U8X8_MSG_CAD_SEND_ARG
    U8X8_MSG_CAD_SEND_DATA
    U8X8_MSG_CAD_END_TRANSFER
    
  The byte interface is there to send 1 byte (8 bits) to the display hardware.
  This layer depends on the hardware of a microcontroller, if a specific hardware 
  should be used (I2C or SPI). 
  If this interface is implemented via software, it may use the GPIO level for sending
  bytes.
    U8X8_MSG_BYTE_INIT
    U8X8_MSG_BYTE_SEND 30
    U8X8_MSG_BYTE_SET_DC 31
    U8X8_MSG_BYTE_START_TRANSFER
    U8X8_MSG_BYTE_END_TRANSFER
    U8X8_MSG_BYTE_SET_I2C_ADR (obsolete)
    U8X8_MSG_BYTE_SET_DEVICE (obsolete)

  GPIO and Delay
    U8X8_MSG_GPIO_INIT
    U8X8_MSG_DELAY_MILLI
    U8X8_MSG_DELAY_10MICRO
    U8X8_MSG_DELAY_100NANO
    U8X8_MSG_DELAY_NANO
*/

#ifndef U8X8_H
#define U8X8_H

/*==========================================*/
/* Global Defines */

/* Undefine this to remove u8x8_SetContrast function */
// #define U8X8_WITH_SET_CONTRAST

/* Define this for an additional user pointer inside the u8x8 data struct */
//#define U8X8_WITH_USER_PTR

/* Undefine this to remove u8x8_SetFlipMode function */
/* 26 May 2016: Obsolete */
//#define U8X8_WITH_SET_FLIP_MODE

/* Select 0 or 1 for the default flip mode. This is not affected by U8X8_WITH_FLIP_MODE */
/* Note: Not all display types support a mirror functon for the frame buffer */
/* 26 May 2016: Obsolete */
//#define U8X8_DEFAULT_FLIP_MODE 0

/*==========================================*/
/* Includes */

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

#if defined(__GNUC__) && defined(__AVR__)
#include <avr/pgmspace.h>
#endif

/*==========================================*/
/* C++ compatible */

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================*/
/* U8G2 internal defines */

/* the following macro returns the first value for the normal mode */
/* or the second argument for the flip mode */

/* 26 May 2016: Obsolete
#if U8X8_DEFAULT_FLIP_MODE == 0
#define U8X8_IF_DEFAULT_NORMAL_OR_FLIP(normal, flipmode) (normal)
#else
#define U8X8_IF_DEFAULT_NORMAL_OR_FLIP(normal, flipmode) (flipmode)
#endif
*/

#ifdef __GNUC__
#define U8X8_NOINLINE      __attribute__((noinline))
#define U8X8_SECTION(name) __attribute__((section(name)))
#define U8X8_UNUSED        __attribute__((unused))
#else
#define U8X8_SECTION(name)
#define U8X8_NOINLINE
#define U8X8_UNUSED
#endif

#if defined(__GNUC__) && defined(__AVR__)
#define U8X8_FONT_SECTION(name) U8X8_SECTION(".progmem." name)
#define u8x8_pgm_read(adr)      pgm_read_byte_near(adr)
#define U8X8_PROGMEM            PROGMEM
#endif

#if defined(ESP8266)
uint8_t u8x8_pgm_read_esp(const uint8_t* addr); /* u8x8_8x8.c */
#define U8X8_FONT_SECTION(name) __attribute__((section(".text." name)))
#define u8x8_pgm_read(adr)      u8x8_pgm_read_esp(adr)
#define U8X8_PROGMEM
#endif

#ifndef U8X8_FONT_SECTION
#define U8X8_FONT_SECTION(name)
#endif

#ifndef u8x8_pgm_read
#ifndef CHAR_BIT
#define u8x8_pgm_read(adr) (*(const uint8_t*)(adr))
#else
#if CHAR_BIT > 8
#define u8x8_pgm_read(adr) ((*(const uint8_t*)(adr)) & 0x0ff)
#else
#define u8x8_pgm_read(adr) (*(const uint8_t*)(adr))
#endif
#endif
#endif

#ifndef U8X8_PROGMEM
#define U8X8_PROGMEM
#endif

#ifdef ARDUINO
#define U8X8_USE_PINS
#endif

/*==========================================*/
/* U8X8 typedefs and data structures */

typedef struct u8x8_struct u8x8_t;
typedef struct u8x8_display_info_struct u8x8_display_info_t;
typedef struct u8x8_tile_struct u8x8_tile_t;

typedef uint8_t (*u8x8_msg_cb)(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
typedef uint16_t (*u8x8_char_cb)(u8x8_t* u8x8, uint8_t b);

//struct u8x8_mcd_struct
//{
//  u8x8_msg_cb cb;		/* current callback function */
//  u8x8_t *u8g2;		/* pointer to the u8g2 parent to minimize the number of args */
//  u8x8_mcd_t *next;
//};

struct u8x8_tile_struct {
    uint8_t* tile_ptr; /* pointer to one or more tiles... should be "const" */
    uint8_t cnt; /* number of tiles */
    uint8_t x_pos; /* tile x position */
    uint8_t y_pos; /* tile x position */
};

struct u8x8_display_info_struct {
    /* == general == */

    uint8_t chip_enable_level; /* UC1601: 0 */
    uint8_t chip_disable_level; /* opposite of chip_enable_level */

    uint8_t post_chip_enable_wait_ns; /* UC1601: 5ns */
    uint8_t pre_chip_disable_wait_ns; /* UC1601: 5ns */
    uint8_t reset_pulse_width_ms; /* UC1601: 0.003ms --> 1ms */
    uint8_t post_reset_wait_ms; /* UC1601: 6ms  */

    /* == SPI interface == */

    /* after SDA has been applied, wait this much time for the SCK data takeover edge */
    /* if this is smaller than sck_pulse_width_ns, then use the value from sck_pulse_width_ns */
    uint8_t sda_setup_time_ns; /* UC1601: 12ns */
    /* the pulse width of the the clock signal, cycle time is twice this value */
    /* max freq is 1/(2*sck_pulse_width_ns) */
    /* AVR: below 70: DIV2, 8 MHz, >= 70 --> 4MHz clock (DIV4) */
    uint8_t sck_pulse_width_ns; /* UC1701: 50ns */

    /* until here we have 8 bytes (uint8_t). Newly introduced for SPI.beginTransaction */
    uint32_t sck_clock_hz;

    /* previous name "sck_takeover_edge" renamed to "spi_mode" */
    /* bit 0 of spi_mode is equal to the value of the previous variable sck_takeover_edge, 20 Aug 16: This is wrong the bit is actually inverted */
    /* SPI has four clock modes: */
    /*   0: clock active high, data out on falling edge, clock default value is zero, takover on rising edge */
    /*   1: clock active high, data out on rising edge, clock default value is zero, takover on falling edge */
    /*   2: clock active low, data out on rising edge */
    /*   3: clock active low, data out on falling edge */
    /* most displays have clock mode 1 */
    uint8_t spi_mode;

    /* == I2C == */
    uint8_t i2c_bus_clock_100kHz; /* UC1601: 1000000000/275 = 37 *100k */

    /* == 8 bit interface == */

    /* how long to wait after all data line are set */
    uint8_t data_setup_time_ns; /* UC1601: 30ns */
    /* write enable pulse width */
    uint8_t write_pulse_width_ns; /* UC1601: 40ns */

    /* == layout == */
    uint8_t tile_width;
    uint8_t tile_height;

    uint8_t default_x_offset; /* default x offset for the display */
    uint8_t flipmode_x_offset; /* x offset, if flip mode is enabled */

    /* pixel width is not used by the u8x8 procedures */
    /* instead it will be used by the u8g2 procedures, because the pixel dimension can */
    /* not always be calculated from the tile_width/_height */
    /* the following conditions must be true: */
    /* pixel_width <= tile_width*8 */
    /* pixel_height <= tile_height*8 */
    uint16_t pixel_width;
    uint16_t pixel_height;
};

/* list of U8x8 pins */
#define U8X8_PIN_D0        0
#define U8X8_PIN_SPI_CLOCK 0
#define U8X8_PIN_D1        1
#define U8X8_PIN_SPI_DATA  1
#define U8X8_PIN_D2        2
#define U8X8_PIN_D3        3
#define U8X8_PIN_D4        4
#define U8X8_PIN_D5        5
#define U8X8_PIN_D6        6
#define U8X8_PIN_D7        7

#define U8X8_PIN_E     8
#define U8X8_PIN_CS    9 /* parallel, SPI */
#define U8X8_PIN_DC    10 /* parallel, SPI */
#define U8X8_PIN_RESET 11 /* parallel, SPI, I2C */

#define U8X8_PIN_I2C_CLOCK 12 /* 1 = Input/high impedance, 0 = drive low */
#define U8X8_PIN_I2C_DATA  13 /* 1 = Input/high impedance, 0 = drive low */

#define U8X8_PIN_CS1 14 /* KS0108 extra chip select */
#define U8X8_PIN_CS2 15 /* KS0108 extra chip select */

#define U8X8_PIN_OUTPUT_CNT 16

#define U8X8_PIN_MENU_SELECT 16
#define U8X8_PIN_MENU_NEXT   17
#define U8X8_PIN_MENU_PREV   18
#define U8X8_PIN_MENU_HOME   19
#define U8X8_PIN_MENU_UP     20
#define U8X8_PIN_MENU_DOWN   21

#define U8X8_PIN_INPUT_CNT 6

#ifdef U8X8_USE_PINS
#define U8X8_PIN_CNT  (U8X8_PIN_OUTPUT_CNT + U8X8_PIN_INPUT_CNT)
#define U8X8_PIN_NONE 255
#endif

struct u8x8_struct {
    const u8x8_display_info_t* display_info;
    u8x8_char_cb next_cb; /*  procedure, which will be used to get the next char from the string */
    u8x8_msg_cb display_cb;
    u8x8_msg_cb cad_cb;
    u8x8_msg_cb byte_cb;
    u8x8_msg_cb gpio_and_delay_cb;
    uint32_t bus_clock; /* can be used by the byte function to store the clock speed of the bus */
    const uint8_t* font;
    uint16_t encoding; /* encoding result for utf8 decoder in next_cb */
    uint8_t x_offset; /* copied from info struct, can be modified in flip mode */
    uint8_t is_font_inverse_mode; /* 0: normal, 1: font glyphs are inverted */
    uint8_t
        i2c_address; /* a valid i2c adr. Initially this is 255, but this is set to something usefull during DISPLAY_INIT */
    /* i2c_address is the address for writing data to the display */
    /* usually, the lowest bit must be zero for a valid address */
    uint8_t i2c_started; /* for i2c interface */
    //uint8_t device_address;	/* OBSOLETE???? - this is the device address, replacement for U8X8_MSG_CAD_SET_DEVICE */
    uint8_t utf8_state; /* number of chars which are still to scan */
    uint8_t gpio_result; /* return value from the gpio call (only for MENU keys at the moment) */
    uint8_t debounce_default_pin_state;
    uint8_t debounce_last_pin_state;
    uint8_t debounce_state;
    uint8_t debounce_result_msg; /* result msg or event after debounce */
#ifdef U8X8_WITH_USER_PTR
    void* user_ptr;
#endif
#ifdef U8X8_USE_PINS
    uint8_t pins
        [U8X8_PIN_CNT]; /* defines a pinlist: Mainly a list of pins for the Arduino Envionment, use U8X8_PIN_xxx to access */
#endif
};

#ifdef U8X8_WITH_USER_PTR
#define u8x8_GetUserPtr(u8x8)    ((u8x8)->user_ptr)
#define u8x8_SetUserPtr(u8x8, p) ((u8x8)->user_ptr = (p))
#endif

#define u8x8_GetCols(u8x8)                ((u8x8)->display_info->tile_width)
#define u8x8_GetRows(u8x8)                ((u8x8)->display_info->tile_height)
#define u8x8_GetI2CAddress(u8x8)          ((u8x8)->i2c_address)
#define u8x8_SetI2CAddress(u8x8, address) ((u8x8)->i2c_address = (address))

#define u8x8_SetGPIOResult(u8x8, val) ((u8x8)->gpio_result = (val))
#define u8x8_GetSPIClockPhase(u8x8) \
    ((u8x8)->display_info->spi_mode & 0x01) /* 0 means rising edge */
#define u8x8_GetSPIClockPolarity(u8x8)     (((u8x8)->display_info->spi_mode & 0x02) >> 1)
#define u8x8_GetSPIClockDefaultLevel(u8x8) (((u8x8)->display_info->spi_mode & 0x02) >> 1)

#define u8x8_GetFontCharWidth(u8x8)  u8x8_pgm_read((u8x8)->font + 2)
#define u8x8_GetFontCharHeight(u8x8) u8x8_pgm_read((u8x8)->font + 3)

#ifdef U8X8_USE_PINS
#define u8x8_SetPin(u8x8, pin, val)      (u8x8)->pins[pin] = (val)
#define u8x8_SetMenuSelectPin(u8x8, val) u8x8_SetPin((u8x8), U8X8_PIN_MENU_SELECT, (val))
#define u8x8_SetMenuNextPin(u8x8, val)   u8x8_SetPin((u8x8), U8X8_PIN_MENU_NEXT, (val))
#define u8x8_SetMenuPrevPin(u8x8, val)   u8x8_SetPin((u8x8), U8X8_PIN_MENU_PREV, (val))
#define u8x8_SetMenuHomePin(u8x8, val)   u8x8_SetPin((u8x8), U8X8_PIN_MENU_HOME, (val))
#define u8x8_SetMenuUpPin(u8x8, val)     u8x8_SetPin((u8x8), U8X8_PIN_MENU_UP, (val))
#define u8x8_SetMenuDownPin(u8x8, val)   u8x8_SetPin((u8x8), U8X8_PIN_MENU_DOWN, (val))
#endif

/*==========================================*/
/* u8log extension for u8x8 and u8g2 */

typedef struct u8log_struct u8log_t;

/* redraw the specified line. */
typedef void (*u8log_cb)(u8log_t* u8log);

struct u8log_struct {
    /* configuration */
    void* aux_data; /* pointer to u8x8 or u8g2 */
    uint8_t width, height; /* size of the terminal */
    u8log_cb cb; /* callback redraw function */
    uint8_t* screen_buffer; /* size must be width*heigh bytes */
    uint8_t is_redraw_line_for_each_char;
    int8_t line_height_offset; /* extra offset for the line height (u8g2 only) */

    /* internal data */
    //uint8_t last_x, last_y;	/* position of the last printed char */
    uint8_t cursor_x, cursor_y; /* position of the cursor, might be off screen */
    uint8_t redraw_line; /* redraw specific line if is_redraw_line is not 0 */
    uint8_t is_redraw_line;
    uint8_t is_redraw_all;
    uint8_t is_redraw_all_required_for_next_nl; /* in nl mode, redraw all instead of current line */
};

/*==========================================*/

/* helper functions */
void u8x8_d_helper_display_setup_memory(u8x8_t* u8x8, const u8x8_display_info_t* display_info);
void u8x8_d_helper_display_init(u8x8_t* u8g2);

/* Display Interface */

/*
  Name: 	U8X8_MSG_DISPLAY_SETUP_MEMORY
  Args:	None
  Tasks:
    1) setup u8g2->display_info
      copy u8g2->display_info->default_x_offset to u8g2->x_offset
      
   usually calls u8x8_d_helper_display_setup_memory()
*/
#define U8X8_MSG_DISPLAY_SETUP_MEMORY 9

/*
  Name: 	U8X8_MSG_DISPLAY_INIT
  Args:	None
  Tasks:

    2) put interface into default state: 
	  execute u8x8_gpio_Init for port directions
	  execute u8x8_cad_Init for default port levels
    3) set CS status (not clear, may be done in cad/byte interface
    4) execute display reset (gpio interface)
    5) send setup sequence to display, do not activate display, disable "power save" will follow 
*/
#define U8X8_MSG_DISPLAY_INIT 10

/*
  Name: 	U8X8_MSG_DISPLAY_SET_POWER_SAVE
  Args:	arg_int: 0: normal mode (RAM is visible on the display), 1: nothing is shown
  Tasks:
    Depending on arg_int, put the display into normal or power save mode.
    Send the corresponding sequence to the display.
    In power save mode, it must be possible to modify the RAM content.
*/
#define U8X8_MSG_DISPLAY_SET_POWER_SAVE 11

/*
  Name: 	U8X8_MSG_DISPLAY_SET_FLIP_MODE
  Args:	arg_int: 0: normal mode, 1: flipped HW screen (180 degree)
  Tasks:
    Reprogramms the display controller to rotate the display by 
    180 degree (arg_int = 1) or not (arg_int = 0)
    This may change u8g2->x_offset if the display is smaller than the controller ram
    This message should only be supported if U8X8_WITH_FLIP_MODE is defined.
*/
#define U8X8_MSG_DISPLAY_SET_FLIP_MODE 13

/*  arg_int: 0..255 contrast value */
#define U8X8_MSG_DISPLAY_SET_CONTRAST 14

/*
  Name: 	U8X8_MSG_DISPLAY_DRAW_TILE
  Args:	
    arg_int: How often to repeat this tile pattern
    arg_ptr: pointer to u8x8_tile_t
        uint8_t *tile_ptr;	pointer to one or more tiles (number is "cnt")
	uint8_t cnt;		number of tiles
	uint8_t x_pos;		first tile x position
	uint8_t y_pos;		first tile y position 
  Tasks:
    One tile has exactly 8 bytes (8x8 pixel monochrome bitmap). 
    The lowest bit of the first byte is the upper left corner
    The highest bit of the first byte is the lower left corner
    The lowest bit of the last byte is the upper right corner
    The highest bit of the last byte is the lower left corner
    "tile_ptr" is the address of a memory area, which contains
    one or more tiles. "cnt" will contain the exact number of
    tiles in the memory areay. The size of the memory area is 8*cnt;
    Multiple tiles in the memory area form a horizontal sequence, this 
    means the first tile is drawn at x_pos/y_pos, the second tile is drawn
    at x_pos+1/y_pos, third at x_pos+2/y_pos.
    "arg_int" tells how often the tile sequence should be repeated:
    For example if "cnt" is two and tile_ptr points to tiles A and B,
    then for arg_int = 3, the following tile sequence will be drawn:
    ABABAB. Totally, cnt*arg_int tiles will be drawn. 
        
*/
#define U8X8_MSG_DISPLAY_DRAW_TILE 15

/*
  Name: 	U8X8_MSG_DISPLAY_REFRESH
  Args:	
    arg_int: -
    arg_ptr: -
  
  This was introduced for the SSD1606 eInk display.
  The problem is, that all RAM access will not appear on the screen
  unless a special command is executed. With this message, this command
  sequence is executed.
  Use
    void u8x8_RefreshDisplay(u8x8_t *u8x8)
  to send the message to the display handler.
*/
#define U8X8_MSG_DISPLAY_REFRESH 16

/*==========================================*/
/* u8x8_setup.c */

uint8_t u8x8_dummy_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

/* 
  Setup u8x8 object itself. This should be the very first function 
  called on the new u8x8 object. After this call, assign the callback
  functions. Optional: Set the pins 
*/

void u8x8_SetupDefaults(u8x8_t* u8x8); /* do not use this, use u8x8_Setup() instead */

void u8x8_Setup(
    u8x8_t* u8x8,
    u8x8_msg_cb display_cb,
    u8x8_msg_cb cad_cb,
    u8x8_msg_cb byte_cb,
    u8x8_msg_cb gpio_and_delay_cb);

/*==========================================*/
/* u8x8_display.c */
uint8_t u8x8_DrawTile(u8x8_t* u8x8, uint8_t x, uint8_t y, uint8_t cnt, uint8_t* tile_ptr);

/* 
  After a call to u8x8_SetupDefaults, 
  setup u8x8 memory structures & inform callbacks 
  This function is also called from u8x8_Setup(), so do not call u8x8_SetupMemory()
  directly, but use u8x8_Setup() instead.
*/
void u8x8_SetupMemory(u8x8_t* u8x8);

/*
  After calling u8x8_SetupMemory()/u8x8_Setup(), init the display hardware itself.
  This will will the first time, u8x8 talks to the display.
  It will init the display, but keep display in power save mode. 
  Usually this command must be followed by u8x8_SetPowerSave() 
*/
void u8x8_InitDisplay(u8x8_t* u8x8);
/* wake up display from power save mode */
void u8x8_SetPowerSave(u8x8_t* u8x8, uint8_t is_enable);
void u8x8_SetFlipMode(u8x8_t* u8x8, uint8_t mode);
void u8x8_SetContrast(u8x8_t* u8x8, uint8_t value);
void u8x8_ClearDisplayWithTile(u8x8_t* u8x8, const uint8_t* buf) U8X8_NOINLINE;
void u8x8_ClearDisplay(u8x8_t* u8x8); // this does not work for u8g2 in some cases
void u8x8_FillDisplay(u8x8_t* u8x8);
void u8x8_RefreshDisplay(
    u8x8_t* u8x8); // make RAM content visible on the display (Dec 16: SSD1606 only)
void u8x8_ClearLine(u8x8_t* u8x8, uint8_t line);

/*==========================================*/
/* Command Arg Data (CAD) Interface */

/*
  U8X8_MSG_CAD_INIT
    no args
    call U8X8_MSG_BYTE_INIT
    setup default values for the I/O lines
*/
#define U8X8_MSG_CAD_INIT 20

#define U8X8_MSG_CAD_SEND_CMD       21
/*  arg_int: cmd byte */
#define U8X8_MSG_CAD_SEND_ARG       22
/*  arg_int: arg byte */
#define U8X8_MSG_CAD_SEND_DATA      23
/* arg_int: expected cs level after processing this msg */
#define U8X8_MSG_CAD_START_TRANSFER 24
/* arg_int: expected cs level after processing this msg */
#define U8X8_MSG_CAD_END_TRANSFER   25
/* arg_int = 0: disable chip, arg_int = 1: enable chip */
//#define U8X8_MSG_CAD_SET_I2C_ADR 26
//#define U8X8_MSG_CAD_SET_DEVICE 27

/* u8g_cad.c */

#define u8x8_cad_Init(u8x8) ((u8x8)->cad_cb((u8x8), U8X8_MSG_CAD_INIT, 0, NULL))

uint8_t u8x8_cad_SendCmd(u8x8_t* u8x8, uint8_t cmd) U8X8_NOINLINE;
uint8_t u8x8_cad_SendArg(u8x8_t* u8x8, uint8_t arg) U8X8_NOINLINE;
uint8_t u8x8_cad_SendMultipleArg(u8x8_t* u8x8, uint8_t cnt, uint8_t arg) U8X8_NOINLINE;
uint8_t u8x8_cad_SendData(u8x8_t* u8x8, uint8_t cnt, uint8_t* data) U8X8_NOINLINE;
uint8_t u8x8_cad_StartTransfer(u8x8_t* u8x8) U8X8_NOINLINE;
uint8_t u8x8_cad_EndTransfer(u8x8_t* u8x8) U8X8_NOINLINE;
void u8x8_cad_vsendf(u8x8_t* u8x8, const char* fmt, va_list va);
void u8x8_SendF(u8x8_t* u8x8, const char* fmt, ...);

/*
#define U8X8_C(c0)				(0x04), (c0)
#define U8X8_CA(c0,a0)			(0x05), (c0), (a0)
#define U8X8_CAA(c0,a0,a1)		(0x06), (c0), (a0), (a1)
#define U8X8_DATA()			(0x10)
#define U8X8_D1(d0)			(0x11), (d0)
*/

#define U8X8_C(c0)      (U8X8_MSG_CAD_SEND_CMD), (c0)
#define U8X8_A(a0)      (U8X8_MSG_CAD_SEND_ARG), (a0)
#define U8X8_CA(c0, a0) (U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0)
#define U8X8_CAA(c0, a0, a1) \
    (U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1)
#define U8X8_CAAA(c0, a0, a1, a2)                                                                \
    (U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1), \
        (U8X8_MSG_CAD_SEND_ARG), (a2)
#define U8X8_CAAAA(c0, a0, a1, a2, a3)                                                           \
    (U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1), \
        (U8X8_MSG_CAD_SEND_ARG), (a2), (U8X8_MSG_CAD_SEND_ARG), (a3)
#define U8X8_AAC(a0, a1, c0) \
    (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1), (U8X8_MSG_CAD_SEND_CMD), (c0)
#define U8X8_D1(d0) (U8X8_MSG_CAD_SEND_DATA), (d0)

#define U8X8_A4(a0, a1, a2, a3) U8X8_A(a0), U8X8_A(a1), U8X8_A(a2), U8X8_A(a3)
#define U8X8_A8(a0, a1, a2, a3, a4, a5, a6, a7) \
    U8X8_A4((a0), (a1), (a2), (a3)), U8X8_A4((a4), (a5), (a6), (a7))

#define U8X8_START_TRANSFER() (U8X8_MSG_CAD_START_TRANSFER)
#define U8X8_END_TRANSFER()   (U8X8_MSG_CAD_END_TRANSFER)
#define U8X8_DLY(m)           (0xfe), (m) /* delay in milli seconds */
#define U8X8_END()            (0xff)

void u8x8_cad_SendSequence(u8x8_t* u8x8, uint8_t const* data);
uint8_t u8x8_cad_empty(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_110(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_001(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_011(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_100(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_st7920_spi(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_ssd13xx_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_ssd13xx_fast_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_st75256_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_ld7032_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_cad_uc16xx_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

/*==========================================*/
/* Byte Interface */

#define U8X8_MSG_BYTE_INIT   U8X8_MSG_CAD_INIT
#define U8X8_MSG_BYTE_SET_DC 32

#define U8X8_MSG_BYTE_SEND U8X8_MSG_CAD_SEND_DATA

#define U8X8_MSG_BYTE_START_TRANSFER U8X8_MSG_CAD_START_TRANSFER
#define U8X8_MSG_BYTE_END_TRANSFER   U8X8_MSG_CAD_END_TRANSFER

//#define U8X8_MSG_BYTE_SET_I2C_ADR U8X8_MSG_CAD_SET_I2C_ADR
//#define U8X8_MSG_BYTE_SET_DEVICE U8X8_MSG_CAD_SET_DEVICE

uint8_t u8x8_byte_SetDC(u8x8_t* u8x8, uint8_t dc) U8X8_NOINLINE;
uint8_t u8x8_byte_SendByte(u8x8_t* u8x8, uint8_t byte) U8X8_NOINLINE;
uint8_t u8x8_byte_SendBytes(u8x8_t* u8x8, uint8_t cnt, uint8_t* data) U8X8_NOINLINE;
uint8_t u8x8_byte_StartTransfer(u8x8_t* u8x8);
uint8_t u8x8_byte_EndTransfer(u8x8_t* u8x8);

uint8_t u8x8_byte_empty(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_byte_4wire_sw_spi(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_byte_8bit_6800mode(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_byte_8bit_8080mode(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_byte_3wire_sw_spi(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
/* uint8_t u8x8_byte_st7920_sw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); */
void u8x8_byte_set_ks0108_cs(u8x8_t* u8x8, uint8_t arg) U8X8_NOINLINE;
uint8_t u8x8_byte_ks0108(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_byte_ssd13xx_sw_i2c(
    u8x8_t* u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void* arg_ptr); /* OBSOLETE! */
uint8_t u8x8_byte_sw_i2c(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_byte_sed1520(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

/*==========================================*/
/* GPIO Interface */

/*
  U8X8_MSG_GPIO_AND_DELAY_INIT
  no args
  setup port directions, do not set IO levels, this is done with BYTE/CAD_INIT
*/
#define U8X8_MSG_GPIO_AND_DELAY_INIT 40

/* arg_int: milliseconds */
#define U8X8_MSG_DELAY_MILLI 41

/* 10MICRO and 100NANO are not used at the moment */
#define U8X8_MSG_DELAY_10MICRO 42
#define U8X8_MSG_DELAY_100NANO 43

#define U8X8_MSG_DELAY_NANO 44
/* delay of one i2c unit, should be 5us for 100K, and 1.25us for 400K */
#define U8X8_MSG_DELAY_I2C  45

#define U8X8_MSG_GPIO(x) (64 + (x))
#ifdef U8X8_USE_PINS
#define u8x8_GetPinIndex(u8x8, msg) ((msg) & 0x3f)
#define u8x8_GetPinValue(u8x8, msg) ((u8x8)->pins[(msg) & 0x3f])
#endif

#define U8X8_MSG_GPIO_D0        U8X8_MSG_GPIO(U8X8_PIN_D0)
#define U8X8_MSG_GPIO_SPI_CLOCK U8X8_MSG_GPIO(U8X8_PIN_SPI_CLOCK)
#define U8X8_MSG_GPIO_D1        U8X8_MSG_GPIO(U8X8_PIN_D1)
#define U8X8_MSG_GPIO_SPI_DATA  U8X8_MSG_GPIO(U8X8_PIN_SPI_DATA)
#define U8X8_MSG_GPIO_D2        U8X8_MSG_GPIO(U8X8_PIN_D2)
#define U8X8_MSG_GPIO_D3        U8X8_MSG_GPIO(U8X8_PIN_D3)
#define U8X8_MSG_GPIO_D4        U8X8_MSG_GPIO(U8X8_PIN_D4)
#define U8X8_MSG_GPIO_D5        U8X8_MSG_GPIO(U8X8_PIN_D5)
#define U8X8_MSG_GPIO_D6        U8X8_MSG_GPIO(U8X8_PIN_D6)
#define U8X8_MSG_GPIO_D7        U8X8_MSG_GPIO(U8X8_PIN_D7)
#define U8X8_MSG_GPIO_E         U8X8_MSG_GPIO(U8X8_PIN_E) // used as E1 for the SED1520
#define U8X8_MSG_GPIO_CS        U8X8_MSG_GPIO(U8X8_PIN_CS) // used as E2 for the SED1520
#define U8X8_MSG_GPIO_DC        U8X8_MSG_GPIO(U8X8_PIN_DC)
#define U8X8_MSG_GPIO_RESET     U8X8_MSG_GPIO(U8X8_PIN_RESET)
#define U8X8_MSG_GPIO_I2C_CLOCK U8X8_MSG_GPIO(U8X8_PIN_I2C_CLOCK)
#define U8X8_MSG_GPIO_I2C_DATA  U8X8_MSG_GPIO(U8X8_PIN_I2C_DATA)

#define U8X8_MSG_GPIO_CS1 U8X8_MSG_GPIO(U8X8_PIN_CS1) /* KS0108 extra chip select */
#define U8X8_MSG_GPIO_CS2 U8X8_MSG_GPIO(U8X8_PIN_CS2) /* KS0108 extra chip select */

/* these message expect the return value in u8x8->gpio_result */
#define U8X8_MSG_GPIO_MENU_SELECT U8X8_MSG_GPIO(U8X8_PIN_MENU_SELECT)
#define U8X8_MSG_GPIO_MENU_NEXT   U8X8_MSG_GPIO(U8X8_PIN_MENU_NEXT)
#define U8X8_MSG_GPIO_MENU_PREV   U8X8_MSG_GPIO(U8X8_PIN_MENU_PREV)
#define U8X8_MSG_GPIO_MENU_HOME   U8X8_MSG_GPIO(U8X8_PIN_MENU_HOME)
#define U8X8_MSG_GPIO_MENU_UP     U8X8_MSG_GPIO(U8X8_PIN_MENU_UP)
#define U8X8_MSG_GPIO_MENU_DOWN   U8X8_MSG_GPIO(U8X8_PIN_MENU_DOWN)

#define u8x8_gpio_Init(u8x8) \
    ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_AND_DELAY_INIT, 0, NULL))

/*
#define u8x8_gpio_SetDC(u8x8, v) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_DC, (v), NULL ))
#define u8x8_gpio_SetCS(u8x8, v) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_CS, (v), NULL ))
#define u8x8_gpio_SetReset(u8x8, v) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_RESET, (v), NULL ))
*/

#define u8x8_gpio_SetDC(u8x8, v)       u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_DC, (v))
#define u8x8_gpio_SetCS(u8x8, v)       u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_CS, (v))
#define u8x8_gpio_SetReset(u8x8, v)    u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_RESET, (v))
#define u8x8_gpio_SetSPIClock(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_SPI_CLOCK, (v))
#define u8x8_gpio_SetSPIData(u8x8, v)  u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_SPI_DATA, (v))
#define u8x8_gpio_SetI2CClock(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_I2C_CLOCK, (v))
#define u8x8_gpio_SetI2CData(u8x8, v)  u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_I2C_DATA, (v))

void u8x8_gpio_call(u8x8_t* u8x8, uint8_t msg, uint8_t arg) U8X8_NOINLINE;

#define u8x8_gpio_Delay(u8x8, msg, dly) u8x8_gpio_call((u8x8), (msg), (dly))
//void u8x8_gpio_Delay(u8x8_t *u8x8, uint8_t msg, uint8_t dly) U8X8_NOINLINE;

/*==========================================*/
/* u8x8_debounce.c */
/* return U8X8_MSG_GPIO_MENU_xxxxx messages */
uint8_t u8x8_GetMenuEvent(u8x8_t* u8x8);

/*==========================================*/
/* u8x8_d_stdio.c */
void u8x8_SetupStdio(u8x8_t* u8x8);

/*==========================================*/
/* u8x8_d_sdl_128x64.c */
void u8x8_Setup_SDL_128x64(u8x8_t* u8x8);
void u8x8_Setup_SDL_240x160(u8x8_t* u8x8);
int u8g_sdl_get_key(void);

/*==========================================*/
/* u8x8_d_tga.c */
void u8x8_Setup_TGA_DESC(u8x8_t* u8x8);
void u8x8_Setup_TGA_LCD(u8x8_t* u8x8);
void tga_save(const char* name);

/*==========================================*/
/* u8x8_d_bitmap.c */
uint8_t u8x8_GetBitmapPixel(u8x8_t* u8x8, uint16_t x, uint16_t y);
void u8x8_SaveBitmapTGA(u8x8_t* u8x8, const char* filename);
void u8x8_SetupBitmap(u8x8_t* u8x8, uint16_t pixel_width, uint16_t pixel_height);
uint8_t u8x8_ConnectBitmapToU8x8(u8x8_t* u8x8);

/*==========================================*/
/* u8x8_d_utf8.c */
void u8x8_Setup_Utf8(u8x8_t* u8x8); /* stdout UTF-8 display */
void utf8_show(void); /* show content of UTF-8 frame buffer */

/*==========================================*/

/* u8x8_setup.c */
uint8_t u8x8_d_null_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

/* u8x8_d_XXX.c */
uint8_t u8x8_d_uc1701_ea_dogs102(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1701_mini12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1305_128x32_noname(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1305_128x32_adafruit(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1305_128x64_adafruit(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_128x64_noname(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_128x64_vcomh0(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_128x64_alt0(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1309_128x64_noname0(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1309_128x64_noname2(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1106_128x64_noname(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1106_128x64_vcomh0(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1106_128x64_winstar(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1106_72x40_wise(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1106_64x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1107_64x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1107_seeed_96x96(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1107_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1107_pimoroni_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1107_seeed_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1108_160x160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sh1122_256x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7920_192x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7920_128x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_128x32_univision(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_128x32_winstar(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_64x48_er(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_48x64_winstar(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_64x32_noname(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_64x32_1f(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_96x16_er(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1306_72x40_er(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ls013b7dh03_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ls027b7dh01_400x240(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ls013b7dh05_144x168(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7511_avd_320x240(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7528_nhd_c160100(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_ea_dogm128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_lm6063(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_64128n(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_ea_dogm132(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_zolen_128x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_nhd_c12832(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_nhd_c12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_jlx12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_lm6059(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_lx12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_erc12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7565_erc12864_alt(
    u8x8_t* u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void* arg_ptr); /* issue #790 */
uint8_t u8x8_d_st7567_pi_132x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7567_jlx12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7567_enh_dg128064(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7567_enh_dg128064i(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7567_64x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7567_os12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7586s_s028hn118a(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7586s_erc240160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st7588_jlx12864(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx256128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_wo256x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx256160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx256160m(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx256160_alt(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx240160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx25664(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx172104(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75256_jlx19296(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_st75320_jlx320240(
    u8x8_t* u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void* arg_ptr); /* https://github.com/olikraus/u8g2/issues/921 */
uint8_t u8x8_d_nt7534_tg12864r(
    u8x8_t* u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void* arg_ptr); /* u8x8_d_st7565.c */
uint8_t u8x8_d_ld7032_60x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_t6963_240x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_t6963_240x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_t6963_128x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_t6963_128x64_alt(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_t6963_160x80(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_t6963_256x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1316_128x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1317_96x96(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1318_128x96(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1318_128x96_xcp(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1322_nhd_256x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1322_nhd_128x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_a2printer_384x240(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sed1330_240x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ra8835_nhd_240x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ra8835_320x240(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1325_nhd_128x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd0323_os128064(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1327_ws_96x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1327_seeed_96x96(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1327_ea_w128128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1327_midas_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1327_ws_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1327_visionox_128x96(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1326_er_256x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1329_128x96_noname(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1601_128x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1604_jlx19264(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1608_erc24064(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1608_erc240120(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1608_240x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1610_ea_dogxl160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1611_ea_dogm240(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1611_ea_dogxl240(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t
    u8x8_d_uc1611_ew50850(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr); /* 240x160 */
uint8_t
    u8x8_d_uc1611_cg160160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr); /* 160x160 */
uint8_t u8x8_d_uc1617_jlx128128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_uc1638_160x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ks0108_128x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ks0108_erm19264(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sbn1661_122x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_sed1520_122x32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_pcd8544_84x48(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_pcf8812_96x65(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_hx1230_96x68(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1606_172x72(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1607_200x200(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1607_v2_200x200(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1607_gd_200x200(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ssd1607_ws_200x200(
    u8x8_t* u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void* arg_ptr); /* issue 637 */
uint8_t u8x8_d_il3820_296x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_il3820_v2_296x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_lc7981_160x80(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_lc7981_160x160(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_lc7981_240x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_lc7981_240x64(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ist3020_erc19264(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_ist7920_128x128(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_max7219_64x8(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_max7219_32x8(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_max7219_16x16(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
uint8_t u8x8_d_max7219_8x8(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

/*==========================================*/
/* u8x8_8x8.c */

uint16_t u8x8_upscale_byte(uint8_t x) U8X8_NOINLINE;

void u8x8_utf8_init(u8x8_t* u8x8);
uint16_t u8x8_ascii_next(u8x8_t* u8x8, uint8_t b);
uint16_t u8x8_utf8_next(u8x8_t* u8x8, uint8_t b);
// the following two functions are replaced by the init/next functions
//uint16_t u8x8_get_encoding_from_utf8_string(const char **str);
//uint16_t u8x8_get_char_from_string(const char **str);

void u8x8_SetFont(u8x8_t* u8x8, const uint8_t* font_8x8);
void u8x8_DrawGlyph(u8x8_t* u8x8, uint8_t x, uint8_t y, uint8_t encoding);
void u8x8_Draw2x2Glyph(u8x8_t* u8x8, uint8_t x, uint8_t y, uint8_t encoding);
void u8x8_Draw1x2Glyph(u8x8_t* u8x8, uint8_t x, uint8_t y, uint8_t encoding);
uint8_t u8x8_DrawString(u8x8_t* u8x8, uint8_t x, uint8_t y, const char* s);
uint8_t
    u8x8_DrawUTF8(u8x8_t* u8x8, uint8_t x, uint8_t y, const char* s); /* return number of glyps */
uint8_t u8x8_Draw2x2String(u8x8_t* u8x8, uint8_t x, uint8_t y, const char* s);
uint8_t u8x8_Draw2x2UTF8(u8x8_t* u8x8, uint8_t x, uint8_t y, const char* s);
uint8_t u8x8_Draw1x2String(u8x8_t* u8x8, uint8_t x, uint8_t y, const char* s);
uint8_t u8x8_Draw1x2UTF8(u8x8_t* u8x8, uint8_t x, uint8_t y, const char* s);
uint8_t u8x8_GetUTF8Len(u8x8_t* u8x8, const char* s);
#define u8x8_SetInverseFont(u8x8, b) (u8x8)->is_font_inverse_mode = (b)

/*==========================================*/
/* itoa procedures */
const char* u8x8_u8toa(uint8_t v, uint8_t d);
const char* u8x8_u16toa(uint16_t v, uint8_t d);
const char* u8x8_utoa(uint16_t v);

/*==========================================*/
/* u8x8_string.c */

uint8_t u8x8_GetStringLineCnt(const char* str); /* return 0 for str==NULL */
const char* u8x8_GetStringLineStart(uint8_t line_idx, const char* str);
void u8x8_CopyStringLine(char* dest, uint8_t line_idx, const char* str);
/* draw one line, consider \t for center */
uint8_t u8x8_DrawUTF8Line(u8x8_t* u8x8, uint8_t x, uint8_t y, uint8_t w, const char* s);
/* draw multiple lines, handle \t */
uint8_t u8x8_DrawUTF8Lines(u8x8_t* u8x8, uint8_t x, uint8_t y, uint8_t w, const char* s);

/*==========================================*/

/* u8x8_selection_list.c */
struct _u8sl_struct {
    uint8_t visible; /* number of visible elements in the menu */
    uint8_t total; /* total number of elements in the menu */
    uint8_t first_pos; /* position of the first visible line */
    uint8_t current_pos; /* current cursor position, starts at 0 */

    uint8_t x; /* u8x8 only, not used in u8g2 */
    uint8_t y; /* u8x8 only, not used in u8g2 */
};
typedef struct _u8sl_struct u8sl_t;

typedef void (*u8x8_sl_cb)(u8x8_t* u8x8, u8sl_t* u8sl, uint8_t idx, const void* aux);

void u8sl_Next(u8sl_t* u8sl);
void u8sl_Prev(u8sl_t* u8sl);

uint8_t u8x8_UserInterfaceSelectionList(
    u8x8_t* u8x8,
    const char* title,
    uint8_t start_pos,
    const char* sl);

/*==========================================*/

/* u8x8_message.c  */
uint8_t u8x8_UserInterfaceMessage(
    u8x8_t* u8x8,
    const char* title1,
    const char* title2,
    const char* title3,
    const char* buttons);

/*==========================================*/
/* u8x8_capture.c */

/* vertical_top memory architecture */
uint8_t u8x8_capture_get_pixel_1(uint16_t x, uint16_t y, uint8_t* dest_ptr, uint8_t tile_width);

/* horizontal right memory architecture */
/* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */
uint8_t u8x8_capture_get_pixel_2(uint16_t x, uint16_t y, uint8_t* dest_ptr, uint8_t tile_width);

void u8x8_capture_write_pbm_pre(
    uint8_t tile_width,
    uint8_t tile_height,
    void (*out)(const char* s));
void u8x8_capture_write_pbm_buffer(
    uint8_t* buffer,
    uint8_t tile_width,
    uint8_t tile_height,
    uint8_t (*get_pixel)(uint16_t x, uint16_t y, uint8_t* dest_ptr, uint8_t tile_width),
    void (*out)(const char* s));

void u8x8_capture_write_xbm_pre(
    uint8_t tile_width,
    uint8_t tile_height,
    void (*out)(const char* s));
void u8x8_capture_write_xbm_buffer(
    uint8_t* buffer,
    uint8_t tile_width,
    uint8_t tile_height,
    uint8_t (*get_pixel)(uint16_t x, uint16_t y, uint8_t* dest_ptr, uint8_t tile_width),
    void (*out)(const char* s));

/*==========================================*/

/* u8x8_input_value.c  */

uint8_t u8x8_UserInterfaceInputValue(
    u8x8_t* u8x8,
    const char* title,
    const char* pre,
    uint8_t* value,
    uint8_t lo,
    uint8_t hi,
    uint8_t digits,
    const char* post);

/*==========================================*/
/* u8log.c */
void u8log_Init(u8log_t* u8log, uint8_t width, uint8_t height, uint8_t* buf);
void u8log_SetCallback(u8log_t* u8log, u8log_cb cb, void* aux_data);
void u8log_SetRedrawMode(u8log_t* u8log, uint8_t is_redraw_line_for_each_char);
void u8log_SetLineHeightOffset(u8log_t* u8log, int8_t line_height_offset);
void u8log_WriteString(u8log_t* u8log, const char* s) U8X8_NOINLINE;
void u8log_WriteChar(u8log_t* u8log, uint8_t c) U8X8_NOINLINE;
void u8log_WriteHex8(u8log_t* u8log, uint8_t b) U8X8_NOINLINE;
void u8log_WriteHex16(u8log_t* u8log, uint16_t v);
void u8log_WriteHex32(u8log_t* u8log, uint32_t v);
void u8log_WriteDec8(u8log_t* u8log, uint8_t v, uint8_t d);
void u8log_WriteDec16(u8log_t* u8log, uint16_t v, uint8_t d);

/*==========================================*/
/* u8log_u8x8.c */
void u8x8_DrawLog(u8x8_t* u8x8, uint8_t x, uint8_t y, u8log_t* u8log);
void u8log_u8x8_cb(u8log_t* u8log);

/*==========================================*/
/* start font list */
extern const uint8_t
    u8x8_font_amstrad_cpc_extended_f[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_f");
extern const uint8_t
    u8x8_font_amstrad_cpc_extended_r[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_r");
extern const uint8_t
    u8x8_font_amstrad_cpc_extended_n[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_n");
extern const uint8_t
    u8x8_font_amstrad_cpc_extended_u[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_u");
extern const uint8_t u8x8_font_5x7_f[] U8X8_FONT_SECTION("u8x8_font_5x7_f");
extern const uint8_t u8x8_font_5x7_r[] U8X8_FONT_SECTION("u8x8_font_5x7_r");
extern const uint8_t u8x8_font_5x7_n[] U8X8_FONT_SECTION("u8x8_font_5x7_n");
extern const uint8_t u8x8_font_5x8_f[] U8X8_FONT_SECTION("u8x8_font_5x8_f");
extern const uint8_t u8x8_font_5x8_r[] U8X8_FONT_SECTION("u8x8_font_5x8_r");
extern const uint8_t u8x8_font_5x8_n[] U8X8_FONT_SECTION("u8x8_font_5x8_n");
extern const uint8_t u8x8_font_8x13_1x2_f[] U8X8_FONT_SECTION("u8x8_font_8x13_1x2_f");
extern const uint8_t u8x8_font_8x13_1x2_r[] U8X8_FONT_SECTION("u8x8_font_8x13_1x2_r");
extern const uint8_t u8x8_font_8x13_1x2_n[] U8X8_FONT_SECTION("u8x8_font_8x13_1x2_n");
extern const uint8_t u8x8_font_8x13B_1x2_f[] U8X8_FONT_SECTION("u8x8_font_8x13B_1x2_f");
extern const uint8_t u8x8_font_8x13B_1x2_r[] U8X8_FONT_SECTION("u8x8_font_8x13B_1x2_r");
extern const uint8_t u8x8_font_8x13B_1x2_n[] U8X8_FONT_SECTION("u8x8_font_8x13B_1x2_n");
extern const uint8_t u8x8_font_7x14_1x2_f[] U8X8_FONT_SECTION("u8x8_font_7x14_1x2_f");
extern const uint8_t u8x8_font_7x14_1x2_r[] U8X8_FONT_SECTION("u8x8_font_7x14_1x2_r");
extern const uint8_t u8x8_font_7x14_1x2_n[] U8X8_FONT_SECTION("u8x8_font_7x14_1x2_n");
extern const uint8_t u8x8_font_7x14B_1x2_f[] U8X8_FONT_SECTION("u8x8_font_7x14B_1x2_f");
extern const uint8_t u8x8_font_7x14B_1x2_r[] U8X8_FONT_SECTION("u8x8_font_7x14B_1x2_r");
extern const uint8_t u8x8_font_7x14B_1x2_n[] U8X8_FONT_SECTION("u8x8_font_7x14B_1x2_n");
extern const uint8_t
    u8x8_font_open_iconic_arrow_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_1x1");
extern const uint8_t
    u8x8_font_open_iconic_check_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_1x1");
extern const uint8_t
    u8x8_font_open_iconic_embedded_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_1x1");
extern const uint8_t
    u8x8_font_open_iconic_play_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_1x1");
extern const uint8_t
    u8x8_font_open_iconic_thing_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_1x1");
extern const uint8_t
    u8x8_font_open_iconic_weather_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_1x1");
extern const uint8_t
    u8x8_font_open_iconic_arrow_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_2x2");
extern const uint8_t
    u8x8_font_open_iconic_check_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_2x2");
extern const uint8_t
    u8x8_font_open_iconic_embedded_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_2x2");
extern const uint8_t
    u8x8_font_open_iconic_play_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_2x2");
extern const uint8_t
    u8x8_font_open_iconic_thing_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_2x2");
extern const uint8_t
    u8x8_font_open_iconic_weather_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_2x2");
extern const uint8_t
    u8x8_font_open_iconic_arrow_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_4x4");
extern const uint8_t
    u8x8_font_open_iconic_check_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_4x4");
extern const uint8_t
    u8x8_font_open_iconic_embedded_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_4x4");
extern const uint8_t
    u8x8_font_open_iconic_play_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_4x4");
extern const uint8_t
    u8x8_font_open_iconic_thing_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_4x4");
extern const uint8_t
    u8x8_font_open_iconic_weather_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_4x4");
extern const uint8_t
    u8x8_font_open_iconic_arrow_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_8x8");
extern const uint8_t
    u8x8_font_open_iconic_check_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_8x8");
extern const uint8_t
    u8x8_font_open_iconic_embedded_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_8x8");
extern const uint8_t
    u8x8_font_open_iconic_play_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_8x8");
extern const uint8_t
    u8x8_font_open_iconic_thing_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_8x8");
extern const uint8_t
    u8x8_font_open_iconic_weather_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_8x8");
extern const uint8_t u8x8_font_profont29_2x3_f[] U8X8_FONT_SECTION("u8x8_font_profont29_2x3_f");
extern const uint8_t u8x8_font_profont29_2x3_r[] U8X8_FONT_SECTION("u8x8_font_profont29_2x3_r");
extern const uint8_t u8x8_font_profont29_2x3_n[] U8X8_FONT_SECTION("u8x8_font_profont29_2x3_n");
extern const uint8_t u8x8_font_artossans8_r[] U8X8_FONT_SECTION("u8x8_font_artossans8_r");
extern const uint8_t u8x8_font_artossans8_n[] U8X8_FONT_SECTION("u8x8_font_artossans8_n");
extern const uint8_t u8x8_font_artossans8_u[] U8X8_FONT_SECTION("u8x8_font_artossans8_u");
extern const uint8_t u8x8_font_artosserif8_r[] U8X8_FONT_SECTION("u8x8_font_artosserif8_r");
extern const uint8_t u8x8_font_artosserif8_n[] U8X8_FONT_SECTION("u8x8_font_artosserif8_n");
extern const uint8_t u8x8_font_artosserif8_u[] U8X8_FONT_SECTION("u8x8_font_artosserif8_u");
extern const uint8_t
    u8x8_font_chroma48medium8_r[] U8X8_FONT_SECTION("u8x8_font_chroma48medium8_r");
extern const uint8_t
    u8x8_font_chroma48medium8_n[] U8X8_FONT_SECTION("u8x8_font_chroma48medium8_n");
extern const uint8_t
    u8x8_font_chroma48medium8_u[] U8X8_FONT_SECTION("u8x8_font_chroma48medium8_u");
extern const uint8_t
    u8x8_font_saikyosansbold8_n[] U8X8_FONT_SECTION("u8x8_font_saikyosansbold8_n");
extern const uint8_t
    u8x8_font_saikyosansbold8_u[] U8X8_FONT_SECTION("u8x8_font_saikyosansbold8_u");
extern const uint8_t u8x8_font_torussansbold8_r[] U8X8_FONT_SECTION("u8x8_font_torussansbold8_r");
extern const uint8_t u8x8_font_torussansbold8_n[] U8X8_FONT_SECTION("u8x8_font_torussansbold8_n");
extern const uint8_t u8x8_font_torussansbold8_u[] U8X8_FONT_SECTION("u8x8_font_torussansbold8_u");
extern const uint8_t u8x8_font_victoriabold8_r[] U8X8_FONT_SECTION("u8x8_font_victoriabold8_r");
extern const uint8_t u8x8_font_victoriabold8_n[] U8X8_FONT_SECTION("u8x8_font_victoriabold8_n");
extern const uint8_t u8x8_font_victoriabold8_u[] U8X8_FONT_SECTION("u8x8_font_victoriabold8_u");
extern const uint8_t
    u8x8_font_victoriamedium8_r[] U8X8_FONT_SECTION("u8x8_font_victoriamedium8_r");
extern const uint8_t
    u8x8_font_victoriamedium8_n[] U8X8_FONT_SECTION("u8x8_font_victoriamedium8_n");
extern const uint8_t
    u8x8_font_victoriamedium8_u[] U8X8_FONT_SECTION("u8x8_font_victoriamedium8_u");
extern const uint8_t u8x8_font_courB18_2x3_f[] U8X8_FONT_SECTION("u8x8_font_courB18_2x3_f");
extern const uint8_t u8x8_font_courB18_2x3_r[] U8X8_FONT_SECTION("u8x8_font_courB18_2x3_r");
extern const uint8_t u8x8_font_courB18_2x3_n[] U8X8_FONT_SECTION("u8x8_font_courB18_2x3_n");
extern const uint8_t u8x8_font_courR18_2x3_f[] U8X8_FONT_SECTION("u8x8_font_courR18_2x3_f");
extern const uint8_t u8x8_font_courR18_2x3_r[] U8X8_FONT_SECTION("u8x8_font_courR18_2x3_r");
extern const uint8_t u8x8_font_courR18_2x3_n[] U8X8_FONT_SECTION("u8x8_font_courR18_2x3_n");
extern const uint8_t u8x8_font_courB24_3x4_f[] U8X8_FONT_SECTION("u8x8_font_courB24_3x4_f");
extern const uint8_t u8x8_font_courB24_3x4_r[] U8X8_FONT_SECTION("u8x8_font_courB24_3x4_r");
extern const uint8_t u8x8_font_courB24_3x4_n[] U8X8_FONT_SECTION("u8x8_font_courB24_3x4_n");
extern const uint8_t u8x8_font_courR24_3x4_f[] U8X8_FONT_SECTION("u8x8_font_courR24_3x4_f");
extern const uint8_t u8x8_font_courR24_3x4_r[] U8X8_FONT_SECTION("u8x8_font_courR24_3x4_r");
extern const uint8_t u8x8_font_courR24_3x4_n[] U8X8_FONT_SECTION("u8x8_font_courR24_3x4_n");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_o_2x2_f[] U8X8_FONT_SECTION(
    "u8x8_font_lucasarts_scumm_subtitle_o_2x2_f");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_o_2x2_r[] U8X8_FONT_SECTION(
    "u8x8_font_lucasarts_scumm_subtitle_o_2x2_r");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_o_2x2_n[] U8X8_FONT_SECTION(
    "u8x8_font_lucasarts_scumm_subtitle_o_2x2_n");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_r_2x2_f[] U8X8_FONT_SECTION(
    "u8x8_font_lucasarts_scumm_subtitle_r_2x2_f");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_r_2x2_r[] U8X8_FONT_SECTION(
    "u8x8_font_lucasarts_scumm_subtitle_r_2x2_r");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_r_2x2_n[] U8X8_FONT_SECTION(
    "u8x8_font_lucasarts_scumm_subtitle_r_2x2_n");
extern const uint8_t u8x8_font_inr21_2x4_f[] U8X8_FONT_SECTION("u8x8_font_inr21_2x4_f");
extern const uint8_t u8x8_font_inr21_2x4_r[] U8X8_FONT_SECTION("u8x8_font_inr21_2x4_r");
extern const uint8_t u8x8_font_inr21_2x4_n[] U8X8_FONT_SECTION("u8x8_font_inr21_2x4_n");
extern const uint8_t u8x8_font_inr33_3x6_f[] U8X8_FONT_SECTION("u8x8_font_inr33_3x6_f");
extern const uint8_t u8x8_font_inr33_3x6_r[] U8X8_FONT_SECTION("u8x8_font_inr33_3x6_r");
extern const uint8_t u8x8_font_inr33_3x6_n[] U8X8_FONT_SECTION("u8x8_font_inr33_3x6_n");
extern const uint8_t u8x8_font_inr46_4x8_f[] U8X8_FONT_SECTION("u8x8_font_inr46_4x8_f");
extern const uint8_t u8x8_font_inr46_4x8_r[] U8X8_FONT_SECTION("u8x8_font_inr46_4x8_r");
extern const uint8_t u8x8_font_inr46_4x8_n[] U8X8_FONT_SECTION("u8x8_font_inr46_4x8_n");
extern const uint8_t u8x8_font_inb21_2x4_f[] U8X8_FONT_SECTION("u8x8_font_inb21_2x4_f");
extern const uint8_t u8x8_font_inb21_2x4_r[] U8X8_FONT_SECTION("u8x8_font_inb21_2x4_r");
extern const uint8_t u8x8_font_inb21_2x4_n[] U8X8_FONT_SECTION("u8x8_font_inb21_2x4_n");
extern const uint8_t u8x8_font_inb33_3x6_f[] U8X8_FONT_SECTION("u8x8_font_inb33_3x6_f");
extern const uint8_t u8x8_font_inb33_3x6_r[] U8X8_FONT_SECTION("u8x8_font_inb33_3x6_r");
extern const uint8_t u8x8_font_inb33_3x6_n[] U8X8_FONT_SECTION("u8x8_font_inb33_3x6_n");
extern const uint8_t u8x8_font_inb46_4x8_f[] U8X8_FONT_SECTION("u8x8_font_inb46_4x8_f");
extern const uint8_t u8x8_font_inb46_4x8_r[] U8X8_FONT_SECTION("u8x8_font_inb46_4x8_r");
extern const uint8_t u8x8_font_inb46_4x8_n[] U8X8_FONT_SECTION("u8x8_font_inb46_4x8_n");
extern const uint8_t u8x8_font_pressstart2p_f[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_f");
extern const uint8_t u8x8_font_pressstart2p_r[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_r");
extern const uint8_t u8x8_font_pressstart2p_n[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_n");
extern const uint8_t u8x8_font_pressstart2p_u[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_u");
extern const uint8_t u8x8_font_pcsenior_f[] U8X8_FONT_SECTION("u8x8_font_pcsenior_f");
extern const uint8_t u8x8_font_pcsenior_r[] U8X8_FONT_SECTION("u8x8_font_pcsenior_r");
extern const uint8_t u8x8_font_pcsenior_n[] U8X8_FONT_SECTION("u8x8_font_pcsenior_n");
extern const uint8_t u8x8_font_pcsenior_u[] U8X8_FONT_SECTION("u8x8_font_pcsenior_u");
extern const uint8_t
    u8x8_font_pxplusibmcgathin_f[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_f");
extern const uint8_t
    u8x8_font_pxplusibmcgathin_r[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_r");
extern const uint8_t
    u8x8_font_pxplusibmcgathin_n[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_n");
extern const uint8_t
    u8x8_font_pxplusibmcgathin_u[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_u");
extern const uint8_t u8x8_font_pxplusibmcga_f[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_f");
extern const uint8_t u8x8_font_pxplusibmcga_r[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_r");
extern const uint8_t u8x8_font_pxplusibmcga_n[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_n");
extern const uint8_t u8x8_font_pxplusibmcga_u[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_u");
extern const uint8_t
    u8x8_font_pxplustandynewtv_f[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_f");
extern const uint8_t
    u8x8_font_pxplustandynewtv_r[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_r");
extern const uint8_t
    u8x8_font_pxplustandynewtv_n[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_n");
extern const uint8_t
    u8x8_font_pxplustandynewtv_u[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_u");
extern const uint8_t
    u8x8_font_px437wyse700a_2x2_f[] U8X8_FONT_SECTION("u8x8_font_px437wyse700a_2x2_f");
extern const uint8_t
    u8x8_font_px437wyse700a_2x2_r[] U8X8_FONT_SECTION("u8x8_font_px437wyse700a_2x2_r");
extern const uint8_t
    u8x8_font_px437wyse700a_2x2_n[] U8X8_FONT_SECTION("u8x8_font_px437wyse700a_2x2_n");
extern const uint8_t
    u8x8_font_px437wyse700b_2x2_f[] U8X8_FONT_SECTION("u8x8_font_px437wyse700b_2x2_f");
extern const uint8_t
    u8x8_font_px437wyse700b_2x2_r[] U8X8_FONT_SECTION("u8x8_font_px437wyse700b_2x2_r");
extern const uint8_t
    u8x8_font_px437wyse700b_2x2_n[] U8X8_FONT_SECTION("u8x8_font_px437wyse700b_2x2_n");

/* end font list */

#ifdef __cplusplus
}
#endif

#endif /* _U8X8_H */
