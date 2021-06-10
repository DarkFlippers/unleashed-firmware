#include "nfc_cli.h"
#include "nfc_types.h"
#include <furi.h>
#include <api-hal.h>

void nfc_cli_init() {
    Cli* cli = furi_record_open("cli");
    cli_add_command(cli, "nfc_detect", nfc_cli_detect, NULL);
    cli_add_command(cli, "nfc_emulate", nfc_cli_emulate, NULL);
    furi_record_close("cli");
}

void nfc_cli_detect(Cli* cli, string_t args, void* context) {
    // Check if nfc worker is not busy
    if(api_hal_nfc_is_busy()) {
        printf("Nfc is busy");
        return;
    }
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    bool cmd_exit = false;
    api_hal_nfc_init();
    api_hal_nfc_exit_sleep();
    printf("Detecting nfc...\r\nPress Ctrl+C to abort\r\n");
    while(!cmd_exit) {
        cmd_exit |= cli_cmd_interrupt_received(cli);
        cmd_exit |= api_hal_nfc_detect(&dev_list, &dev_cnt, 100, true);
        if(dev_cnt > 0) {
            printf("Found %d devices\r\n", dev_cnt);
            for(uint8_t i = 0; i < dev_cnt; i++) {
                printf("%d found: %s ", i + 1, nfc_get_dev_type(dev_list[i].type));
                if(dev_list[i].type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                    printf("type: %s, ", nfc_get_nfca_type(dev_list[i].dev.nfca.type));
                }
                printf("UID length: %d, UID:", dev_list[i].nfcidLen);
                for(uint8_t j = 0; j < dev_list[i].nfcidLen; j++) {
                    printf("%02X", dev_list[i].nfcid[j]);
                }
                printf("\r\n");
            }
        }
        osDelay(50);
    }
    api_hal_nfc_deactivate();
}

void nfc_cli_emulate(Cli* cli, string_t args, void* context) {
    // Check if nfc worker is not busy
    if(api_hal_nfc_is_busy()) {
        printf("Nfc is busy");
        return;
    }

    api_hal_nfc_init();
    api_hal_nfc_exit_sleep();
    printf("Emulating NFC-A Type: T2T UID: CF72D440 SAK: 20 ATQA: 00/04\r\n");
    printf("Press Ctrl+C to abort\r\n");

    while(!cli_cmd_interrupt_received(cli)) {
        if(api_hal_nfc_listen(100)) {
            printf("Reader detected\r\n");
            api_hal_nfc_deactivate();
        }
        osDelay(50);
    }
    api_hal_nfc_deactivate();
}
