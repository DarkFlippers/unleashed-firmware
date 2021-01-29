#include "applications.h"

#ifdef APP_TEST
void flipper_test_app(void* p);
#endif

void application_blink(void* p);
void application_uart_write(void* p);
void application_input_dump(void* p);
void display_u8g2(void* p);
void u8g2_example(void* p);
void input_task(void* p);
void menu_task(void* p);
void coreglitch_demo_0(void* p);
void u8g2_qrcode(void* p);
void fatfs_list(void* p);
void gui_task(void* p);
void backlight_control(void* p);
void irda(void* p);
void app_loader(void* p);
void cc1101_workaround(void* p);
void lf_rfid_workaround(void* p);
void nfc_task(void* p);
void dolphin_task(void* p);
void power_task(void* p);
void bt_task(void* p);
void sd_card_test(void* p);
void application_vibro(void* p);
void app_gpio_test(void* p);
void app_ibutton(void* p);
void cli_task(void* p);
void music_player(void* p);
void sdnfc(void* p);
void floopper_bloopper(void* p);
void sd_filesystem(void* p);

const FuriApplication FLIPPER_SERVICES[] = {
#ifdef APP_DISPLAY
    {.app = display_u8g2, .name = "display_u8g2", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_CLI
    {.app = cli_task, .name = "cli_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_INPUT
    {.app = input_task, .name = "input_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_GUI
    {.app = backlight_control,
     .name = "backlight_control",
     .stack_size = 1024,
     .icon = A_Plugins_14},
    {.app = gui_task, .name = "gui_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_MENU
    {.app = menu_task, .name = "menu_task", .stack_size = 1024, .icon = A_Plugins_14},
    {.app = app_loader, .name = "app_loader", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_SD_FILESYSTEM
    {.app = sd_filesystem, .name = "sd_filesystem", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_DOLPHIN
    {.app = dolphin_task, .name = "dolphin_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_POWER
    {.app = power_task, .name = "power_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_BT
    {.app = bt_task, .name = "bt_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_CC1101
    {.app = cc1101_workaround,
     .name = "cc1101 workaround",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef APP_LF_RFID
    {.app = lf_rfid_workaround,
     .name = "lf rfid workaround",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef APP_IRDA
    {.app = irda, .name = "irda", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_NFC
    {.app = nfc_task, .name = "nfc_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_TEST
    {.app = flipper_test_app, .name = "test app", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_QRCODE
    {.app = u8g2_qrcode, .name = "u8g2_qrcode", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_FATFS
    {.app = fatfs_list, .name = "fatfs_list", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_DISPLAY
    {.app = u8g2_example, .name = "u8g2_example", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_SPEAKER_DEMO
    {.app = coreglitch_demo_0,
     .name = "coreglitch_demo_0",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef APP_SD_TEST
    {.app = sd_card_test, .name = "sd_card_test", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef APP_MUSIC_PLAYER
    {.app = music_player, .name = "music player", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_IBUTTON
    {.app = app_ibutton, .name = "ibutton", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_GPIO_DEMO
    {.app = app_gpio_test, .name = "gpio test", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_FLOOPPER_BLOOPPER
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .stack_size = 1024, .icon = A_Games_14},
#endif

#ifdef APP_SDNFC
    {.app = sdnfc, .name = "sdnfc", .stack_size = 1024, .icon = A_Plugins_14},
#endif
};

size_t FLIPPER_SERVICES_size() {
    return sizeof(FLIPPER_SERVICES) / sizeof(FuriApplication);
}

// Main menu APP
const FuriApplication FLIPPER_APPS[] = {
#ifdef BUILD_CC1101
    {.app = cc1101_workaround, .name = "Sub-1 GHz", .stack_size = 1024, .icon = A_Sub1ghz_14},
#endif

#ifdef BUILD_LF_RFID
    {.app = lf_rfid_workaround, .name = "125 kHz RFID", .stack_size = 1024, .icon = A_125khz_14},
#endif

#ifdef BUILD_IRDA
    {.app = irda, .name = "Infrared", .stack_size = 1024, .icon = A_Infrared_14},
#endif

#ifdef BUILD_IBUTTON
    {.app = app_ibutton, .name = "iButton", .stack_size = 1024, .icon = A_iButton_14},
#endif

#ifdef BUILD_GPIO_DEMO
    {.app = app_gpio_test, .name = "GPIO", .stack_size = 1024, .icon = A_GPIO_14},
#endif
};

size_t FLIPPER_APPS_size() {
    return sizeof(FLIPPER_APPS) / sizeof(FuriApplication);
}

// Plugin menu
const FuriApplication FLIPPER_PLUGINS[] = {
#ifdef BUILD_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef BUILD_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef BUILD_SPEAKER_DEMO
    {.app = coreglitch_demo_0,
     .name = "coreglitch_demo_0",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef BUILD_SD_TEST
    {.app = sd_card_test, .name = "sd_card_test", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef BUILD_VIBRO_DEMO
    {.app = application_vibro,
     .name = "application_vibro",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef BUILD_MUSIC_PLAYER
    {.app = music_player, .name = "music player", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef BUILD_FLOOPPER_BLOOPPER
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .stack_size = 1024, .icon = A_Games_14},
#endif

#ifdef BUILD_SDNFC
    {.app = sdnfc, .name = "sdnfc", .stack_size = 1024, .icon = A_Plugins_14},
#endif
};

size_t FLIPPER_PLUGINS_size() {
    return sizeof(FLIPPER_PLUGINS) / sizeof(FuriApplication);
}
