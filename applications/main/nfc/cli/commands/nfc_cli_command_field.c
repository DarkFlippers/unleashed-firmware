#include "nfc_cli_command_field.h"

#include <furi_hal_nfc.h>

static void nfc_cli_field(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    furi_hal_nfc_low_power_mode_stop();
    furi_hal_nfc_poller_field_on();

    printf("Field is on. Don't leave device in this mode for too long.\r\n");
    printf("Press Ctrl+C to abort\r\n");

    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_delay_ms(50);
    }

    furi_hal_nfc_low_power_mode_start();
}

const NfcCliCommandDescriptor field_cmd = {
    .name = "field",
    .description = "Turns NFC field on",
    .callback = nfc_cli_field,
    .action_count = 0,
    .actions = NULL,
};
