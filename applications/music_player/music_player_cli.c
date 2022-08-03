#include <furi.h>
#include <cli/cli.h>
#include <storage/storage.h>
#include "music_player_worker.h"

static void music_player_cli(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    MusicPlayerWorker* music_player_worker = music_player_worker_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        if(storage_common_stat(storage, string_get_cstr(args), NULL) == FSE_OK) {
            if(!music_player_worker_load(music_player_worker, string_get_cstr(args))) {
                printf("Failed to open file %s\r\n", string_get_cstr(args));
                break;
            }
        } else {
            if(!music_player_worker_load_rtttl_from_string(
                   music_player_worker, string_get_cstr(args))) {
                printf("Argument is not a file or RTTTL\r\n");
                break;
            }
        }

        printf("Press CTRL+C to stop\r\n");
        music_player_worker_set_volume(music_player_worker, 1.0f);
        music_player_worker_start(music_player_worker);
        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(50);
        }
        music_player_worker_stop(music_player_worker);
    } while(0);

    furi_record_close(RECORD_STORAGE);
    music_player_worker_free(music_player_worker);
}

void music_player_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "music_player", CliCommandFlagDefault, music_player_cli, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(music_player_cli);
#endif
}
