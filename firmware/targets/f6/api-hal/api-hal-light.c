#include <api-hal-light.h>
#include <lp5562.h>

#define LED_CURRENT_RED     50
#define LED_CURRENT_GREEN   50
#define LED_CURRENT_BLUE    50
#define LED_CURRENT_WHITE   150

void api_hal_light_init() {
    lp5562_reset();

    lp5562_set_channel_current(LP5562ChannelRed, LED_CURRENT_RED);
    lp5562_set_channel_current(LP5562ChannelGreen, LED_CURRENT_GREEN);
    lp5562_set_channel_current(LP5562ChannelBlue, LED_CURRENT_BLUE);
    lp5562_set_channel_current(LP5562ChannelWhite, LED_CURRENT_WHITE);

    lp5562_set_channel_value(LP5562ChannelRed, 0x00);
    lp5562_set_channel_value(LP5562ChannelGreen, 0x00);
    lp5562_set_channel_value(LP5562ChannelBlue, 0x00);
    lp5562_set_channel_value(LP5562ChannelWhite, 0x00);

    lp5562_enable();
    lp5562_configure();
    FURI_LOG_I("FuriHalLight", "Init OK");
}

void api_hal_light_set(Light light, uint8_t value) {
    switch(light) {
        case LightRed:
            lp5562_set_channel_value(LP5562ChannelRed, value);
        break;
        case LightGreen:
            lp5562_set_channel_value(LP5562ChannelGreen, value);
        break;
        case LightBlue:
            lp5562_set_channel_value(LP5562ChannelBlue, value);
        break;
        case LightBacklight:
            lp5562_set_channel_value(LP5562ChannelWhite, value);
        break;
        default:
        break;
    }
}