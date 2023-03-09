#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <u8g2_glue.h>
#include <assets_icons.h>

#define COUNTER_VALUE (136U)

static void flipper_boot_recovery_draw_splash(u8g2_t* fb, size_t progress) {
    if(progress < COUNTER_VALUE) {
        // Fill the progress bar while the progress is going down
        u8g2_SetDrawColor(fb, 0x01);
        u8g2_DrawRFrame(fb, 59, 41, 69, 8, 2);
        size_t width = (COUNTER_VALUE - progress) * 68 / COUNTER_VALUE;
        u8g2_DrawBox(fb, 60, 42, width, 6);
    } else {
        u8g2_SetDrawColor(fb, 0x00);
        u8g2_DrawRBox(fb, 59, 41, 69, 8, 2);
    }

    u8g2_SendBuffer(fb);
}

void flipper_boot_recovery_exec() {
    u8g2_t* fb = malloc(sizeof(u8g2_t));
    u8g2_Setup_st756x_flipper(fb, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);
    u8g2_InitDisplay(fb);

    furi_hal_compress_icon_init();
    uint8_t* splash_data = NULL;
    furi_hal_compress_icon_decode(icon_get_data(&I_Erase_pin_128x64), &splash_data);

    u8g2_ClearBuffer(fb);
    u8g2_SetDrawColor(fb, 0x01);

    // Draw the recovery picture
    u8g2_DrawXBM(fb, 0, 0, 128, 64, splash_data);
    u8g2_SendBuffer(fb);
    u8g2_SetPowerSave(fb, 0);

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
