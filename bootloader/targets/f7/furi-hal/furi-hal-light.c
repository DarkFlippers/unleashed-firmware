#include <furi-hal-light.h>
#include <lp5562.h>

#define LED_CURRENT_RED 50
#define LED_CURRENT_GREEN 50
#define LED_CURRENT_BLUE 50
#define LED_CURRENT_WHITE 150

void furi_hal_light_init() {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);

    lp5562_reset(&furi_hal_i2c_handle_power);

    lp5562_set_channel_current(&furi_hal_i2c_handle_power, LP5562ChannelRed, LED_CURRENT_RED);
    lp5562_set_channel_current(&furi_hal_i2c_handle_power, LP5562ChannelGreen, LED_CURRENT_GREEN);
    lp5562_set_channel_current(&furi_hal_i2c_handle_power, LP5562ChannelBlue, LED_CURRENT_BLUE);
    lp5562_set_channel_current(&furi_hal_i2c_handle_power, LP5562ChannelWhite, LED_CURRENT_WHITE);

    lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelRed, 0x00);
    lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelGreen, 0x00);
    lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelBlue, 0x00);
    lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelWhite, 0x00);

    lp5562_enable(&furi_hal_i2c_handle_power);
    lp5562_configure(&furi_hal_i2c_handle_power);

    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}

void furi_hal_light_set(Light light, uint8_t value) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_power);
    switch(light) {
    case LightRed:
        lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelRed, value);
        break;
    case LightGreen:
        lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelGreen, value);
        break;
    case LightBlue:
        lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelBlue, value);
        break;
    case LightBacklight:
        lp5562_set_channel_value(&furi_hal_i2c_handle_power, LP5562ChannelWhite, value);
        break;
    default:
        break;
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_power);
}