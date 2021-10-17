#include "u8g2_glue.h"

#include <furi-hal.h>

static FuriHalSpiDevice* u8g2_periphery_display = NULL;

uint8_t u8g2_gpio_and_delay_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    switch(msg) {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        /* HAL initialization contains all what we need so we can skip this part. */
        break;
    case U8X8_MSG_DELAY_MILLI:
        delay(arg_int);
        break;
    case U8X8_MSG_DELAY_10MICRO:
        delay_us(10);
        break;
    case U8X8_MSG_DELAY_100NANO:
        asm("nop");
        break;
    case U8X8_MSG_GPIO_RESET:
        hal_gpio_write(&gpio_display_rst, arg_int);
        break;
    default:
        return 0;
    }

    return 1;
}

uint8_t u8x8_hw_spi_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    switch(msg) {
    case U8X8_MSG_BYTE_SEND:
        furi_hal_spi_bus_tx(u8g2_periphery_display->bus, (uint8_t*)arg_ptr, arg_int, 10000);
        break;
    case U8X8_MSG_BYTE_SET_DC:
        hal_gpio_write(&gpio_display_di, arg_int);
        break;
    case U8X8_MSG_BYTE_INIT:
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        furi_assert(u8g2_periphery_display == NULL);
        u8g2_periphery_display =
            (FuriHalSpiDevice*)furi_hal_spi_device_get(FuriHalSpiDeviceIdDisplay);
        hal_gpio_write(u8g2_periphery_display->chip_select, false);
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        furi_assert(u8g2_periphery_display);
        hal_gpio_write(u8g2_periphery_display->chip_select, true);
        furi_hal_spi_device_return(u8g2_periphery_display);
        u8g2_periphery_display = NULL;
        break;
    default:
        return 0;
    }

    return 1;
}

static const uint8_t u8x8_d_st7565_powersave0_seq[] = {
    U8X8_START_TRANSFER(),              /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a4),                      /* all pixel off, issue 142 */
    U8X8_C(0x0af),                      /* display on */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                          /* end of sequence */
};

static const uint8_t u8x8_d_st7565_powersave1_seq[] = {
    U8X8_START_TRANSFER(),              /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0ae),                      /* display off */
    U8X8_C(0x0a5),                      /* enter powersafe: all pixel on, issue 142 */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                          /* end of sequence */
};

static const uint8_t u8x8_d_st7565_flip0_seq[] = {
    U8X8_START_TRANSFER(),              /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a1),                      /* segment remap a0/a1*/
    U8X8_C(0x0c0),                      /* c0: scan dir normal, c8: reverse */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                          /* end of sequence */
};

static const uint8_t u8x8_d_st7565_flip1_seq[] = {
    U8X8_START_TRANSFER(),              /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a0),                      /* segment remap a0/a1*/
    U8X8_C(0x0c8),                      /* c0: scan dir normal, c8: reverse */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                          /* end of sequence */
};

static const u8x8_display_info_t u8x8_st756x_128x64_display_info = {
    .chip_enable_level = 0,
    .chip_disable_level = 1,
    .post_chip_enable_wait_ns = 150,    /* st7565 datasheet, table 26, tcsh */
    .pre_chip_disable_wait_ns = 50,     /* st7565 datasheet, table 26, tcss */
    .reset_pulse_width_ms = 1,
    .post_reset_wait_ms = 1,
    .sda_setup_time_ns = 50,            /* st7565 datasheet, table 26, tsds */
    .sck_pulse_width_ns = 120,          /* half of cycle time (100ns according to datasheet), AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
    .sck_clock_hz = 4000000UL,          /* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
    .spi_mode = 0,                      /* active high, rising edge */
    .i2c_bus_clock_100kHz = 4,
    .data_setup_time_ns = 40,           /* st7565 datasheet, table 24, tds8 */
    .write_pulse_width_ns = 80,         /* st7565 datasheet, table 24, tcclw */
    .tile_width = 16,                   /* width of 16*8=128 pixel */
    .tile_height = 8,
    .default_x_offset = 0,
    .flipmode_x_offset = 4,
    .pixel_width = 128,
    .pixel_height = 64
};

uint8_t u8x8_d_st7565_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    uint8_t x, c;
    uint8_t *ptr;

    switch(msg) {
        case U8X8_MSG_DISPLAY_DRAW_TILE:
            u8x8_cad_StartTransfer(u8x8);

            x = ((u8x8_tile_t *)arg_ptr)->x_pos;
            x *= 8;
            x += u8x8->x_offset;
            u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
            u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
            u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));

            c = ((u8x8_tile_t *)arg_ptr)->cnt;
            c *= 8;
            ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
            /* 
                The following if condition checks the hardware limits of the st7565 
                controller: It is not allowed to write beyond the display limits.
                This is in fact an issue within flip mode.
            */
            if ( c + x > 132u ) {
                c = 132u;
                c -= x;
            }

            do {
                u8x8_cad_SendData(u8x8, c, ptr);    /* note: SendData can not handle more than 255 bytes */
                arg_int--;
            } while( arg_int > 0 );

            u8x8_cad_EndTransfer(u8x8);
            break;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
            if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st7565_powersave0_seq);
            else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st7565_powersave1_seq);
            break;
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:
            u8x8_cad_StartTransfer(u8x8);
            u8x8_cad_SendCmd(u8x8, 0x081 );
            u8x8_cad_SendArg(u8x8, arg_int >> 2 );  /* st7565 has range from 0 to 63 */
            u8x8_cad_EndTransfer(u8x8);
            break;
#endif
        default:
            return 0;
    }
    return 1;
}

static const uint8_t u8x8_d_st756x_erc_init_seq[] = {
    U8X8_START_TRANSFER(),
    U8X8_C(0x0e2),        // soft reset
    U8X8_C(0xA3),         // CMD_SET_BIAS_7
    U8X8_C(0xA0),         // CMD_SET_ADC_NORMAL
    U8X8_C(0xC8),         // CMD_SET_COM_REVERSE
    U8X8_C(0x40),         // CMD_SET_DISP_START_LINE
    U8X8_C(0x28 | 0x4),   // CMD_SET_POWER_CONTROL | 0x4
    U8X8_DLY(50), 
    U8X8_C(0x28 | 0x6),   // CMD_SET_POWER_CONTROL | 0x6
    U8X8_DLY(50), 
    U8X8_C(0x28 | 0x7),   // CMD_SET_POWER_CONTROL | 0x7
    U8X8_DLY(50), 
    U8X8_C(0x20 | 0x6),   // CMD_SET_RESISTOR_RATIO | 0x6
    U8X8_END_TRANSFER(),
    U8X8_END()            // end of sequence
};

uint8_t u8x8_d_st756x_erc(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    /* call common procedure first and handle messages there */
    if (u8x8_d_st7565_common(u8x8, msg, arg_int, arg_ptr) == 0) {
        /* msg not handled, then try here */
        switch(msg){
            case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st756x_128x64_display_info);
            break;
        case U8X8_MSG_DISPLAY_INIT:
            u8x8_d_helper_display_init(u8x8);
            u8x8_cad_SendSequence(u8x8, u8x8_d_st756x_erc_init_seq);
            break;
        case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
            if ( arg_int == 0 ) {
                u8x8_cad_SendSequence(u8x8, u8x8_d_st7565_flip1_seq);
                u8x8->x_offset = u8x8->display_info->default_x_offset;
            } else {
                u8x8_cad_SendSequence(u8x8, u8x8_d_st7565_flip0_seq);
                u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
            }   
            break;
        default:
            /* msg unknown */
            return 0;
        }
    }
    return 1;
}

void u8g2_Setup_st756x_erc(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb) {
    uint8_t tile_buf_height;
    uint8_t *buf;
    u8g2_SetupDisplay(u8g2, u8x8_d_st756x_erc, u8x8_cad_001, byte_cb, gpio_and_delay_cb);
    buf = u8g2_m_16_8_f(&tile_buf_height);
    u8g2_SetupBuffer(u8g2, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, rotation);
}
