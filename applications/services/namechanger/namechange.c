#include <furi_hal.h>
#include <cli/cli.h>
#include <cli/cli_vcp.h>
#include <storage/storage.h>

#include <toolbox/namechanger.h>
#include <bt/bt_service/bt.h>

int32_t namechange_on_system_start(void* p) {
    UNUSED(p);
    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) {
        return 0;
    }

    // Wait for all required services to start and create their records
    uint8_t timeout = 0;
    while(!furi_record_exists(RECORD_CLI) || !furi_record_exists(RECORD_BT) ||
          !furi_record_exists(RECORD_STORAGE)) {
        timeout++;
        if(timeout > 250) {
            return 0;
        }
        furi_delay_ms(5);
    }

    // Hehe bad code now here, bad bad bad, very bad, bad example, dont take it, make it better

    if(NameChanger_Init()) {
        Cli* cli = furi_record_open(RECORD_CLI);
        cli_session_close(cli);
        furi_delay_ms(2); // why i added delays here
        cli_session_open(cli, &cli_vcp);
        furi_record_close(RECORD_CLI);

        furi_delay_ms(3);
        Bt* bt = furi_record_open(RECORD_BT);
        if(!bt_set_profile(bt, BtProfileSerial)) {
            //FURI_LOG_D(TAG, "Failed to touch bluetooth to name change");
        }
        furi_record_close(RECORD_BT);
        bt = NULL;
        furi_delay_ms(3);
    }

    return 0;
}