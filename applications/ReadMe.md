# Structure

## debug 

Applications for factory testing the Flipper.

- `accessor`            - Wiegand server 
- `battery_test_app`    - Battery debug app
- `blink_test`          - LED blinker   
- `bt_debug_app`        - BT test app. Requires full BT stack installed
- `display_test`        - Various display tests & tweaks
- `file_browser_test`   - Test UI for file picker
- `keypad_test`         - Keypad test
- `lfrfid_debug`        - LF RFID debug tool
- `text_box_test`       - UI tests
- `uart_echo`           - UART mode test 
- `unit_tests`          - Unit tests
- `usb_mouse`           - USB HID test
- `usb_test`            - Other USB tests
- `vibro_test`          - Vibro test


## main

Applications for main Flipper menu.

- `archive`             - Archive and file manager 
- `bad_usb`             - Bad USB application
- `gpio`                - GPIO application: includes USART bridge and GPIO control
- `ibutton`             - iButton application, onewire keys and more
- `infrared`            - Infrared application, controls your IR devices
- `lfrfid`              - LF RFID application
- `nfc`                 - NFC application, HF rfid, EMV and etc
- `subghz`              - SubGhz application, 433 fobs and etc
- `u2f`                 - U2F Application


## External

External applications deployed to SD Card

- `clock`               - Clock application
- `dap_link`            - DAP Link OnChip debugger
- `hid_app`             - USB/BT Remote controller
- `music_player`        - Music player app (demo)
- `nfc_magic`           - NFC MFC Magic card application
- `picopass`            - Picopass reader / writer
- `signal_generator`    - Signal generator app: PWM and clock generator
- `snake_game`          - Snake game application
- `spi_mem_manager`     - SPI Memory reader / flasher
- `weather_station`     - SubGHz weather station

## services

Background services providing system APIs to applications.

- `applications.h`      - Firmware application list header

- `bt`                  - BLE service and application
- `cli`                 - Console service and API
- `crypto`              - Crypto cli tools
- `desktop`             - Desktop service
- `dialogs`             - Dialogs service: GUI Dialogs for your app
- `dolphin`             - Dolphin service and supplementary apps
- `gui`                 - GUI service and API
- `input`               - Input service
- `loader`              - Application loader service
- `notification`        - Notification service 
- `power`               - Power service
- `rpc`                 - RPC service and API
- `storage`             - Storage service, internal + sdcard


## settings

Small applications providing configuration for basic firmware and its services.

- `about`               - Small About application that shows flipper info
- `bt_settings_app`     - Bluetooth options
- `desktop_settings`    - Desktop configuration
- `dolphin_passport`    - Dolphin passport app
- `notification_settings` - LCD brightness, sound volume, etc configuration
- `power_settings_app`  - Basic power options
- `storage_settings`    - Storage settings app
- `system`              - System settings


## system

Utility apps not visible in other menus.

- `storage_move_to_sd`  - Data migration tool for internal storage
- `updater`             - Update service & application
