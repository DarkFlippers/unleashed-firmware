#include "namechanger.h"
#include <furi_hal.h>
#include <furi_hal_version.h>
#include <cli/cli.h>
#include <cli/cli_vcp.h>
#include <bt/bt_service/bt.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#define TAG "NameChanger"

static bool namechanger_init() {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    // Kostil + velosiped = top ficha
    uint8_t timeout = 0;
    while(timeout < 11) {
        if(storage_sd_status(storage) == FSE_OK) break;
        furi_delay_ms(250);
        timeout++;
        /*if(timeout == 10) {
            // Failed to init namechanger, SD card not ready
            furi_record_close(RECORD_STORAGE);
            return false;
        }*/
    }

    FuriString* str = furi_string_alloc();
    FlipperFormat* file = flipper_format_file_alloc(storage);

    bool res = false;

    do {
        uint32_t version;
        if(!flipper_format_file_open_existing(file, NAMECHANGER_PATH)) break;
        if(!flipper_format_read_header(file, str, &version)) break;
        if(furi_string_cmp_str(str, NAMECHANGER_HEADER)) break;
        if(version != NAMECHANGER_VERSION) break;

        if(!flipper_format_read_string(file, "Name", str)) break;
        // Check for size
        size_t temp_string_size = furi_string_size(str);
        if(temp_string_size > (size_t)8) break;
        if(temp_string_size < (size_t)2) break;

        // Check for forbidden characters
        const char* name_ptr = furi_string_get_cstr(str);
        bool chars_check_failed = false;

        for(; *name_ptr; ++name_ptr) {
            const char c = *name_ptr;
            if((c < '0' || c > '9') && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) {
                chars_check_failed = true;
                break;
            }
        }

        if(chars_check_failed) break;

        // If all checks was good we can set the name
        version_set_custom_name(NULL, strdup(furi_string_get_cstr(str)));
        furi_hal_version_set_name(version_get_custom_name(NULL));

        res = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(str);

    return res;
}

int32_t namechanger_on_system_start(void* p) {
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

    if(namechanger_init()) {
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