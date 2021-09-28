#include "applications.h"
#include <assets_icons.h>

// Services
extern int32_t bt_srv(void* p);
extern int32_t cli_srv(void* p);
extern int32_t dialogs_srv(void* p);
extern int32_t dolphin_srv(void* p);
extern int32_t gui_srv(void* p);
extern int32_t input_srv(void* p);
extern int32_t loader_srv(void* p);
extern int32_t notification_srv(void* p);
extern int32_t power_observer_srv(void* p);
extern int32_t power_srv(void* p);
extern int32_t storage_srv(void* p);
extern int32_t desktop_srv(void* p);

// Apps
extern int32_t accessor_app(void* p);
extern int32_t archive_app(void* p);
extern int32_t blink_test_app(void* p);
extern int32_t flipper_test_app(void* p);
extern int32_t gpio_test_app(void* p);
extern int32_t ibutton_app(void* p);
extern int32_t irda_app(void* p);
extern int32_t irda_monitor_app(void* p);
extern int32_t keypad_test_app(void* p);
extern int32_t lfrfid_app(void* p);
extern int32_t lfrfid_debug_app(void* p);
extern int32_t nfc_app(void* p);
extern int32_t scened_app(void* p);
extern int32_t storage_test_app(void* p);
extern int32_t subghz_app(void* p);
extern int32_t vibro_test_app(void* p);
extern int32_t bt_debug_app(void* p);

// Plugins
extern int32_t music_player_app(void* p);

// On system start hooks declaration
extern void bt_cli_init();
extern void crypto_cli_init();
extern void ibutton_cli_init();
extern void irda_cli_init();
extern void lfrfid_cli_init();
extern void nfc_cli_init();
extern void storage_cli_init();
extern void subghz_cli_init();
extern void power_cli_init();

// Settings
extern int32_t notification_settings_app(void* p);
extern int32_t storage_settings_app(void* p);
extern int32_t bt_settings_app(void* p);
extern int32_t desktop_settings_app(void* p);
extern int32_t about_settings_app(void* p);
extern int32_t power_settings_app(void* p);

const FlipperApplication FLIPPER_SERVICES[] = {
/* Services */
#ifdef SRV_BT
    {.app = bt_srv, .name = "BT", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_CLI
    {.app = cli_srv, .name = "Cli", .stack_size = 4096, .icon = NULL},
#endif

#ifdef SRV_DIALOGS
    {.app = dialogs_srv, .name = "Dialogs", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_DOLPHIN
    {.app = dolphin_srv, .name = "Dolphin", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_DESKTOP
    {.app = desktop_srv, .name = "Desktop", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_GUI
    {.app = gui_srv, .name = "Gui", .stack_size = 8192, .icon = NULL},
#endif

#ifdef SRV_INPUT
    {.app = input_srv, .name = "Input", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_LOADER
    {.app = loader_srv, .name = "Loader", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_NOTIFICATION
    {.app = notification_srv, .name = "Notification", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_POWER
    {.app = power_srv, .name = "Power", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_POWER_OBSERVER
    {.app = power_observer_srv, .name = "PowerObserver", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_STORAGE
    {.app = storage_srv, .name = "Storage", .stack_size = 4096, .icon = NULL},
#endif
};

const size_t FLIPPER_SERVICES_COUNT = sizeof(FLIPPER_SERVICES) / sizeof(FlipperApplication);

// Main menu APP
const FlipperApplication FLIPPER_APPS[] = {

#ifdef APP_SUBGHZ
    {.app = subghz_app, .name = "Sub-GHz", .stack_size = 2048, .icon = &A_Sub1ghz_14},
#endif

#ifdef APP_NFC
    {.app = nfc_app, .name = "NFC", .stack_size = 4096, .icon = &A_NFC_14},
#endif

#ifdef APP_LF_RFID
    {.app = lfrfid_app, .name = "125 kHz RFID", .stack_size = 2048, .icon = &A_125khz_14},
#endif

#ifdef APP_IRDA
    {.app = irda_app, .name = "Infrared", .stack_size = 1024 * 3, .icon = &A_Infrared_14},
#endif

#ifdef APP_IBUTTON
    {.app = ibutton_app, .name = "iButton", .stack_size = 2048, .icon = &A_iButton_14},
#endif

#ifdef APP_GPIO_TEST
    {.app = gpio_test_app, .name = "GPIO", .stack_size = 1024, .icon = &A_GPIO_14},
#endif

};

const size_t FLIPPER_APPS_COUNT = sizeof(FLIPPER_APPS) / sizeof(FlipperApplication);

// On system start hooks
const FlipperOnStartHook FLIPPER_ON_SYSTEM_START[] = {
#ifdef SRV_CLI
    crypto_cli_init,
#endif

#ifdef APP_IRDA
    irda_cli_init,
#endif

#ifdef APP_NFC
    nfc_cli_init,
#endif

#ifdef APP_SUBGHZ
    subghz_cli_init,
#endif

#ifdef APP_LF_RFID
    lfrfid_cli_init,
#endif

#ifdef APP_IBUTTON
    ibutton_cli_init,
#endif

#ifdef SRV_BT
    bt_cli_init,
#endif

#ifdef SRV_POWER
    power_cli_init,
#endif

#ifdef SRV_STORAGE
    storage_cli_init,
#endif
};

const size_t FLIPPER_ON_SYSTEM_START_COUNT =
    sizeof(FLIPPER_ON_SYSTEM_START) / sizeof(FlipperOnStartHook);

// Plugin menu
const FlipperApplication FLIPPER_PLUGINS[] = {

#ifdef APP_MUSIC_PLAYER
    {.app = music_player_app, .name = "Music Player", .stack_size = 1024, .icon = &A_Plugins_14},
#endif
};

const size_t FLIPPER_PLUGINS_COUNT = sizeof(FLIPPER_PLUGINS) / sizeof(FlipperApplication);

// Plugin menu
const FlipperApplication FLIPPER_DEBUG_APPS[] = {
#ifdef APP_BLINK
    {.app = blink_test_app, .name = "Blink Test", .stack_size = 1024, .icon = &A_Plugins_14},
#endif

#ifdef APP_VIBRO_DEMO
    {.app = vibro_test_app, .name = "Vibro Test", .stack_size = 1024, .icon = &A_Plugins_14},
#endif

#ifdef APP_KEYPAD_TEST
    {.app = keypad_test_app, .name = "Keypad Test", .stack_size = 1024, .icon = &A_Plugins_14},
#endif

#ifdef APP_ACCESSOR
    {.app = accessor_app, .name = "Accessor", .stack_size = 4096, .icon = &A_Plugins_14},
#endif

#ifdef APP_UNIT_TESTS
    {.app = flipper_test_app, .name = "Unit Tests", .stack_size = 1024, .icon = &A_Plugins_14},
#endif

#ifdef APP_IRDA_MONITOR
    {.app = irda_monitor_app, .name = "Irda Monitor", .stack_size = 1024, .icon = &A_Plugins_14},
#endif

#ifdef APP_SCENED
    {.app = scened_app, .name = "Templated Scene", .stack_size = 1024, .icon = &A_Plugins_14},
#endif

#ifdef APP_LF_RFID
    {.app = lfrfid_debug_app, .name = "LF-RFID Debug", .stack_size = 1024, .icon = &A_125khz_14},
#endif

#ifdef SRV_BT
    {.app = bt_debug_app, .name = "Bluetooth Debug", .stack_size = 1024, .icon = NULL},
#endif
};

const size_t FLIPPER_DEBUG_APPS_COUNT = sizeof(FLIPPER_DEBUG_APPS) / sizeof(FlipperApplication);

#ifdef APP_ARCHIVE
const FlipperApplication FLIPPER_ARCHIVE =
    {.app = archive_app, .name = "Archive", .stack_size = 4096, .icon = &A_FileManager_14};
#endif

// Settings menu
const FlipperApplication FLIPPER_SETTINGS_APPS[] = {
#ifdef SRV_BT
    {.app = bt_settings_app, .name = "Bluetooth", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_NOTIFICATION
    {.app = notification_settings_app,
     .name = "LCD and notifications",
     .stack_size = 1024,
     .icon = NULL},
#endif

#ifdef SRV_STORAGE
    {.app = storage_settings_app, .name = "Storage", .stack_size = 2048, .icon = NULL},
#endif

#ifdef SRV_POWER
    {.app = power_settings_app, .name = "Power", .stack_size = 1024, .icon = NULL},
#endif

#ifdef SRV_DESKTOP
    {.app = desktop_settings_app, .name = "Desktop", .stack_size = 1024, .icon = NULL},
#endif

#ifdef APP_ABOUT
    {.app = about_settings_app, .name = "About", .stack_size = 1024, .icon = NULL},
#endif
};

const size_t FLIPPER_SETTINGS_APPS_COUNT =
    sizeof(FLIPPER_SETTINGS_APPS) / sizeof(FlipperApplication);
