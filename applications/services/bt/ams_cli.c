#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <toolbox/args.h>
#include <toolbox/pipe.h>
#include <ble/ble.h>
#include <ams_client_i.h>
#include <gap.h>

static void ams_cli(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    FuriString* sub = furi_string_alloc();
    if(args_read_string_and_trim(args, sub)) {
        if(furi_string_cmp_str(sub, "start") == 0) {
            bool ok = ams_client_start(gap_get_connection_handle());
            printf("start: %s\r\n", ok ? "discovery started" : "failed");
        } else if(furi_string_cmp_str(sub, "test") == 0) {
            bool ok = ams_client_start_test(gap_get_connection_handle());
            printf("test start: %s\r\n", ok ? "discovery started" : "failed");
        } else if(furi_string_cmp_str(sub, "cmd") == 0) {
            int id = 0;
            if(args_read_int_and_trim(args, &id) && id >= 0 && id <= 6) {
                printf(
                    "cmd %d: %s\r\n",
                    id,
                    ams_client_command((AmsCommand)id) ? "sent" : "not ready");
            } else {
                printf("usage: ams cmd <0 play|1 pause|2 toggle|3 next|4 prev|5 vol+|6 vol->\r\n");
            }
        }
    }
    furi_string_free(sub);
    AmsInfo info;
    ams_client_get(&info);
    printf(
        "state: %s, error 0x%02X, updates %u, connection 0x%04X\r\n",
        ams_client_state_name(info.state),
        info.error,
        info.updates,
        gap_get_connection_handle());
    printf(
        "player: %s | playback: %s | queue: %u/%u\r\n",
        info.player,
        info.playback,
        info.queue_index + 1,
        info.queue_count);
    printf("artist: %s\r\ntitle: %s\r\n", info.artist, info.title);
}

void ams_cli_on_system_start(void) {
#ifdef SRV_CLI
    CliRegistry* registry = furi_record_open(RECORD_CLI);
    cli_registry_add_command(registry, "ams", CliCommandFlagParallelSafe, ams_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(ams_cli);
#endif
}
