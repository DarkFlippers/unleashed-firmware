#include "applications.h"

#ifdef APP_TEST
int32_t flipper_test_app(void* p);
#endif

int32_t application_blink(void* p);
int32_t application_uart_write(void* p);
int32_t application_input_dump(void* p);
int32_t u8g2_example(void* p);
int32_t input_task(void* p);
int32_t menu_task(void* p);
int32_t coreglitch_demo_0(void* p);
int32_t u8g2_qrcode(void* p);
int32_t gui_task(void* p);
int32_t backlight_control(void* p);
int32_t irda(void* p);
int32_t app_loader(void* p);
int32_t cc1101_workaround(void* p);
int32_t lf_rfid_workaround(void* p);
int32_t nfc_task(void* p);
int32_t dolphin_task(void* p);
int32_t power_task(void* p);
int32_t bt_task(void* p);
int32_t sd_card_test(void* p);
int32_t application_vibro(void* p);
int32_t app_gpio_test(void* p);
int32_t app_ibutton(void* p);
int32_t cli_task(void* p);
int32_t music_player(void* p);
int32_t sdnfc(void* p);
int32_t floopper_bloopper(void* p);
int32_t sd_filesystem(void* p);
int32_t app_subghz(void* p);
int32_t gui_test(void* p);
int32_t keypad_test(void* p);

const FlipperApplication FLIPPER_SERVICES[] = {
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
    // TODO: fix stack size when sd api will be in separate thread
    {.app = gui_task, .name = "gui_task", .stack_size = 8192, .icon = A_Plugins_14},
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
    {.app = app_ibutton, .name = "ibutton", .stack_size = 4096, .icon = A_Plugins_14},
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

#ifdef APP_GUI_TEST
    {.app = gui_test, .name = "gui_test", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_SUBGHZ
    {.app = app_subghz, .name = "app_subghz", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_KEYPAD_TEST
    {.app = keypad_test, .name = "keypad_test", .icon = A_Plugins_14},
#endif
};

const size_t FLIPPER_SERVICES_COUNT = sizeof(FLIPPER_SERVICES) / sizeof(FlipperApplication);

// Main menu APP
const FlipperApplication FLIPPER_APPS[] = {
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

const size_t FLIPPER_APPS_COUNT = sizeof(FLIPPER_APPS) / sizeof(FlipperApplication);

// Plugin menu
const FlipperApplication FLIPPER_PLUGINS[] = {
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

#ifdef BUILD_GUI_TEST
    {.app = gui_test, .name = "gui_test", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef BUILD_SUBGHZ
    {.app = app_subghz, .name = "app_subghz", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef BUILD_KEYPAD_TEST
    {.app = keypad_test, .name = "keypad_test", .icon = A_Plugins_14},
#endif

};

const size_t FLIPPER_PLUGINS_COUNT = sizeof(FLIPPER_PLUGINS) / sizeof(FlipperApplication);
