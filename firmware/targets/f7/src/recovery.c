#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <u8g2_glue.h>
#include <assets_icons.h>

#define COUNTER_VALUE (100U)

static void flipper_boot_recovery_draw_splash(u8g2_t* fb, size_t progress) {
    u8g2_ClearBuffer(fb);
    u8g2_SetDrawColor(fb, 0x01);

    u8g2_SetFont(fb, u8g2_font_helvB08_tr);
    u8g2_DrawStr(fb, 2, 8, "PIN and Factory Reset");
    u8g2_SetFont(fb, u8g2_font_haxrcorp4089_tr);
    u8g2_DrawStr(fb, 2, 21, "Hold Right to confirm");
    u8g2_DrawStr(fb, 2, 31, "Press Down to cancel");

    if(progress < COUNTER_VALUE) {
        size_t width = progress / (COUNTER_VALUE / 100);
        u8g2_DrawBox(fb, 14 + (50 - width / 2), 54, width, 3);
    }

    u8g2_SetPowerSave(fb, 0);
    u8g2_SendBuffer(fb);
}

void flipper_boot_recovery_exec() {
    u8g2_t* fb = malloc(sizeof(u8g2_t));
    u8g2_Setup_st756x_flipper(fb, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);
    u8g2_InitDisplay(fb);

    size_t counter = COUNTER_VALUE;
    while(counter) {
        if(!furi_hal_gpio_read(&gpio_button_down)) {
            break;
        }

        if(!furi_hal_gpio_read(&gpio_button_right)) {
            counter--;
        } else {
            counter = COUNTER_VALUE;
        }

        flipper_boot_recovery_draw_splash(fb, counter);
    }

    if(!counter) {
        furi_hal_rtc_set_flag(FuriHalRtcFlagFactoryReset);
        furi_hal_rtc_set_pin_fails(0);
        furi_hal_rtc_reset_flag(FuriHalRtcFlagLock);
    }
}
