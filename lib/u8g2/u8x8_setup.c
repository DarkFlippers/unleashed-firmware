/*

  u8x8_setup.c

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

*/


#include "u8x8.h"

/* universal dummy callback, which will be default for all callbacks */
uint8_t u8x8_dummy_cb(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  /* the dummy callback will not handle any message and will fail for all messages */
  return 0;
}


static const u8x8_display_info_t u8x8_null_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 0,
  /* pre_chip_disable_wait_ns = */ 0,
  /* reset_pulse_width_ms = */ 0, 
  /* post_reset_wait_ms = */ 0, 
  /* sda_setup_time_ns = */ 0,		
  /* sck_pulse_width_ns = */ 0,	/* half of cycle time (100ns according to datasheet), AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 0,
  /* write_pulse_width_ns = */ 0,
  /* tile_width = */ 1,		/* 8x8 */
  /* tile_hight = */ 1,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 8,
  /* pixel_height = */ 8
};


/* a special null device */
uint8_t u8x8_d_null_cb(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_null_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      break;
  }
  /* the null device callback will succeed for all messages */
  return 1;
}


/*
  Description:
    Setup u8x8
  Args:
    u8x8	An empty u8x8 structure
*/
void u8x8_SetupDefaults(u8x8_t *u8x8)
{
    u8x8->display_info = NULL;
    u8x8->display_cb = u8x8_dummy_cb;
    u8x8->cad_cb = u8x8_dummy_cb;
    u8x8->byte_cb = u8x8_dummy_cb;
    u8x8->gpio_and_delay_cb = u8x8_dummy_cb;
    u8x8->is_font_inverse_mode = 0;
    //u8x8->device_address = 0;
    u8x8->utf8_state = 0;		/* also reset by u8x8_utf8_init */
    u8x8->bus_clock = 0;		/* issue 769 */
    u8x8->i2c_address = 255;
    u8x8->debounce_default_pin_state = 255;	/* assume all low active buttons */
  
#ifdef U8X8_USE_PINS 
  {
    uint8_t i;
    for( i = 0; i < U8X8_PIN_CNT; i++ )
      u8x8->pins[i] = U8X8_PIN_NONE;
  }
#endif
}


/*
  Description:
    Setup u8x8 and assign the callback function. The dummy 
    callback "u8x8_dummy_cb" can be used, if no callback is required.
    This setup will not communicate with the display itself.
    Use u8x8_InitDisplay() to send the startup code to the Display.
  Args:
    u8x8				An empty u8x8 structure
    display_cb			Display/controller specific callback function
    cad_cb				Display controller specific communication callback function
    byte_cb			Display controller/communication specific callback funtion
    gpio_and_delay_cb	Environment specific callback function

*/
void u8x8_Setup(u8x8_t *u8x8, u8x8_msg_cb display_cb, u8x8_msg_cb cad_cb, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
  /* setup defaults and reset pins to U8X8_PIN_NONE */
  u8x8_SetupDefaults(u8x8);

  /* setup specific callbacks */
  u8x8->display_cb = display_cb;
  u8x8->cad_cb = cad_cb;
  u8x8->byte_cb = byte_cb;
  u8x8->gpio_and_delay_cb = gpio_and_delay_cb;

  /* setup display info */
  u8x8_SetupMemory(u8x8);
}

