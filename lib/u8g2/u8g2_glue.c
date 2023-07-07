#include "u8g2_glue.h"

#include <furi_hal.h>

#define CONTRAST_ERC 31
#define CONTRAST_MGG 31

uint8_t u8g2_gpio_and_delay_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    UNUSED(u8x8);
    UNUSED(arg_ptr);
    switch(msg) {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        /* HAL initialization contains all what we need so we can skip this part. */
        break;
    case U8X8_MSG_DELAY_MILLI:
        furi_delay_ms(arg_int);
        break;
    case U8X8_MSG_DELAY_10MICRO:
        furi_delay_us(10);
        break;
    case U8X8_MSG_DELAY_100NANO:
        asm("nop");
        break;
    case U8X8_MSG_GPIO_RESET:
        furi_hal_gpio_write(&gpio_display_rst_n, arg_int);
        break;
    default:
        return 0;
    }

    return 1;
}

uint8_t u8x8_hw_spi_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    UNUSED(u8x8);
    switch(msg) {
    case U8X8_MSG_BYTE_SEND:
        furi_hal_spi_bus_tx(&furi_hal_spi_bus_handle_display, (uint8_t*)arg_ptr, arg_int, 10000);
        break;
    case U8X8_MSG_BYTE_SET_DC:
        furi_hal_gpio_write(&gpio_display_di, arg_int);
        break;
    case U8X8_MSG_BYTE_INIT:
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        furi_hal_spi_acquire(&furi_hal_spi_bus_handle_display);
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        furi_hal_spi_release(&furi_hal_spi_bus_handle_display);
        break;
    default:
        return 0;
    }

    return 1;
}

#define ST756X_CMD_ON_OFF 0b10101110 /**< 0:0 Switch Display ON/OFF: last bit */
#define ST756X_CMD_SET_LINE 0b01000000 /**< 0:0 Set Start Line: last 6 bits  */
#define ST756X_CMD_SET_PAGE 0b10110000 /**< 0:0 Set Page address: last 4 bits */
#define ST756X_CMD_SET_COLUMN_MSB 0b00010000 /**< 0:0 Set Column MSB: last 4 bits */
#define ST756X_CMD_SET_COLUMN_LSB 0b00000000 /**< 0:0 Set Column LSB: last 4 bits */
#define ST756X_CMD_SEG_DIRECTION 0b10100000 /**< 0:0 Reverse scan direction of SEG: last bit */
#define ST756X_CMD_INVERSE_DISPLAY 0b10100110 /**< 0:0 Invert display: last bit */
#define ST756X_CMD_ALL_PIXEL_ON 0b10100100 /**< 0:0 Set all pixel on: last bit */
#define ST756X_CMD_BIAS_SELECT 0b10100010 /**< 0:0 Select 1/9(0) or 1/7(1) bias: last bit */
#define ST756X_CMD_R_M_W 0b11100000 /**< 0:0 Enter Read Modify Write mode: read+0, write+1 */
#define ST756X_CMD_END 0b11101110 /**< 0:0 Exit Read Modify Write mode */
#define ST756X_CMD_RESET 0b11100010 /**< 0:0 Software Reset */
#define ST756X_CMD_COM_DIRECTION 0b11000000 /**< 0:0 Com direction reverse: +0b1000 */
#define ST756X_CMD_POWER_CONTROL 0b00101000 /**< 0:0 Power control: last 3 bits VB:VR:VF */
#define ST756X_CMD_REGULATION_RATIO 0b00100000 /**< 0:0 Regulation resistor ration: last 3bits */
#define ST756X_CMD_SET_EV 0b10000001 /**< 0:0 Set electronic volume: 5 bits in next byte */
#define ST756X_CMD_SET_BOOSTER \
    0b11111000 /**< 0:0 Set Booster level, 4X(0) or 5X(1): last bit in next byte */
#define ST756X_CMD_NOP 0b11100011 /**< 0:0 No operation */

static const uint8_t u8x8_d_st756x_powersave0_seq[] = {
    U8X8_START_TRANSFER(), /* enable chip, delay is part of the transfer start */
    U8X8_C(ST756X_CMD_ALL_PIXEL_ON | 0b0), /* all pixel off */
    U8X8_C(ST756X_CMD_ON_OFF | 0b1), /* display on */
    U8X8_END_TRANSFER(), /* disable chip */
    U8X8_END() /* end of sequence */
};

static const uint8_t u8x8_d_st756x_powersave1_seq[] = {
    U8X8_START_TRANSFER(), /* enable chip, delay is part of the transfer start */
    U8X8_C(ST756X_CMD_ON_OFF | 0b0), /* display off */
    U8X8_C(ST756X_CMD_ALL_PIXEL_ON | 0b1), /* all pixel on */
    U8X8_END_TRANSFER(), /* disable chip */
    U8X8_END() /* end of sequence */
};

static const uint8_t u8x8_d_st756x_flip0_seq[] = {
    U8X8_START_TRANSFER(), /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a1), /* segment remap a0/a1*/
    U8X8_C(0x0c0), /* c0: scan dir normal, c8: reverse */
    U8X8_END_TRANSFER(), /* disable chip */
    U8X8_END() /* end of sequence */
};

static const uint8_t u8x8_d_st756x_flip1_seq[] = {
    U8X8_START_TRANSFER(), /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a0), /* segment remap a0/a1*/
    U8X8_C(0x0c8), /* c0: scan dir normal, c8: reverse */
    U8X8_END_TRANSFER(), /* disable chip */
    U8X8_END() /* end of sequence */
};

static const u8x8_display_info_t u8x8_st756x_128x64_display_info = {
    .chip_enable_level = 0,
    .chip_disable_level = 1,
    .post_chip_enable_wait_ns = 150, /* st7565 datasheet, table 26, tcsh */
    .pre_chip_disable_wait_ns = 50, /* st7565 datasheet, table 26, tcss */
    .reset_pulse_width_ms = 1,
    .post_reset_wait_ms = 1,
    .sda_setup_time_ns = 50, /* st7565 datasheet, table 26, tsds */
    .sck_pulse_width_ns =
        120, /* half of cycle time (100ns according to datasheet), AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
    .sck_clock_hz =
        4000000UL, /* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
    .spi_mode = 0, /* active high, rising edge */
    .i2c_bus_clock_100kHz = 4,
    .data_setup_time_ns = 40, /* st7565 datasheet, table 24, tds8 */
    .write_pulse_width_ns = 80, /* st7565 datasheet, table 24, tcclw */
    .tile_width = 16, /* width of 16*8=128 pixel */
    .tile_height = 8,
    .default_x_offset = 0,
    .flipmode_x_offset = 4,
    .pixel_width = 128,
    .pixel_height = 64};

uint8_t u8x8_d_st756x_common(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    uint8_t x, c;
    uint8_t* ptr;

    switch(msg) {
    case U8X8_MSG_DISPLAY_DRAW_TILE:
        u8x8_cad_StartTransfer(u8x8);

        x = ((u8x8_tile_t*)arg_ptr)->x_pos;
        x *= 8;
        x += u8x8->x_offset;
        u8x8_cad_SendCmd(u8x8, 0x010 | (x >> 4));
        u8x8_cad_SendCmd(u8x8, 0x000 | ((x & 15)));
        u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t*)arg_ptr)->y_pos));

        c = ((u8x8_tile_t*)arg_ptr)->cnt;
        c *= 8;
        ptr = ((u8x8_tile_t*)arg_ptr)->tile_ptr;
        /* 
                The following if condition checks the hardware limits of the st7565 
                controller: It is not allowed to write beyond the display limits.
                This is in fact an issue within flip mode.
            */
        if(c + x > 132u) {
            c = 132u;
            c -= x;
        }

        do {
            u8x8_cad_SendData(
                u8x8, c, ptr); /* note: SendData can not handle more than 255 bytes */
            arg_int--;
        } while(arg_int > 0);

        u8x8_cad_EndTransfer(u8x8);
        break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
        if(arg_int == 0)
            u8x8_cad_SendSequence(u8x8, u8x8_d_st756x_powersave0_seq);
        else
            u8x8_cad_SendSequence(u8x8, u8x8_d_st756x_powersave1_seq);
        break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
        u8x8_cad_StartTransfer(u8x8);
        u8x8_cad_SendCmd(u8x8, ST756X_CMD_SET_EV);
        u8x8_cad_SendArg(u8x8, arg_int >> 2); /* st7565 has range from 0 to 63 */
        u8x8_cad_EndTransfer(u8x8);
        break;
#endif
    default:
        return 0;
    }
    return 1;
}

void u8x8_d_st756x_init(u8x8_t* u8x8, uint8_t contrast, uint8_t regulation_ratio, bool bias) {
    contrast = contrast & 0b00111111;
    regulation_ratio = regulation_ratio & 0b111;

    u8x8_cad_StartTransfer(u8x8);
    // Reset
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_RESET);
    // Bias: 1/7(0b1) or 1/9(0b0)
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_BIAS_SELECT | bias);
    // Page, Line and Segment config
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_SEG_DIRECTION);
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_COM_DIRECTION | 0b1000);
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_SET_LINE);
    // Set Regulation Ratio
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_REGULATION_RATIO | regulation_ratio);
    // Set EV
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_SET_EV);
    u8x8_cad_SendArg(u8x8, contrast);
    // Enable power
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_POWER_CONTROL | 0b111);

    u8x8_cad_EndTransfer(u8x8);
}

void u8x8_d_st756x_set_contrast(u8x8_t* u8x8, int8_t contrast_offset) {
    uint8_t contrast = (furi_hal_version_get_hw_display() == FuriHalVersionDisplayMgg) ?
                           CONTRAST_MGG :
                           CONTRAST_ERC;
    contrast += contrast_offset;
    contrast = contrast & 0b00111111;

    u8x8_cad_StartTransfer(u8x8);
    u8x8_cad_SendCmd(u8x8, ST756X_CMD_SET_EV);
    u8x8_cad_SendArg(u8x8, contrast);
    u8x8_cad_EndTransfer(u8x8);
}

uint8_t u8x8_d_st756x_flipper(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    /* call common procedure first and handle messages there */
    if(u8x8_d_st756x_common(u8x8, msg, arg_int, arg_ptr) == 0) {
        /* msg not handled, then try here */
        switch(msg) {
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st756x_128x64_display_info);
            break;
        case U8X8_MSG_DISPLAY_INIT:
            u8x8_d_helper_display_init(u8x8);
            FuriHalVersionDisplay display = furi_hal_version_get_hw_display();
            if(display == FuriHalVersionDisplayMgg) {
                /* MGG v0+(ST7567)
                 * EV = 32
                 * RR = V0 / ((1 - (63 - EV) / 162) * 2.1)
                 * RR = 10 / ((1 - (63 - 32) / 162) * 2.1) ~= 5.88 is 6 (0b110)
                 * Bias = 1/9 (false)
                 */
                u8x8_d_st756x_init(u8x8, CONTRAST_MGG, 0b110, false);
            } else {
                /* ERC v1(ST7565) and v2(ST7567)
                 * EV = 33
                 * RR = V0 / ((1 - (63 - EV) / 162) * 2.1)
                 * RR = 9.3 / ((1 - (63 - 32) / 162) * 2.1) ~= 5.47 is 5.5 (0b101)
                 * Bias = 1/9 (false)
                 */
                u8x8_d_st756x_init(u8x8, CONTRAST_ERC, 0b101, false);
            }
            break;
        case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
            if(arg_int == 0) {
                u8x8_cad_SendSequence(u8x8, u8x8_d_st756x_flip1_seq);
                u8x8->x_offset = u8x8->display_info->default_x_offset;
            } else {
                u8x8_cad_SendSequence(u8x8, u8x8_d_st756x_flip0_seq);
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

void u8g2_Setup_st756x_flipper(
    u8g2_t* u8g2,
    const u8g2_cb_t* rotation,
    u8x8_msg_cb byte_cb,
    u8x8_msg_cb gpio_and_delay_cb) {
    uint8_t tile_buf_height;
    uint8_t* buf;
    u8g2_SetupDisplay(u8g2, u8x8_d_st756x_flipper, u8x8_cad_001, byte_cb, gpio_and_delay_cb);
    buf = u8g2_m_16_8_f(&tile_buf_height);
    u8g2_SetupBuffer(u8g2, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, rotation);
}
