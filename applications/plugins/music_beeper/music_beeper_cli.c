#include <furi.h>
#include <cli/cli.h>
#include <storage/storage.h>
#include "music_beeper_worker.h"

static void music_beeper_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    MusicBeeperWorker* music_beeper_worker = music_beeper_worker_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        if(storage_common_stat(storage, furi_string_get_cstr(args), NULL) == FSE_OK) {
            if(!music_beeper_worker_load(music_beeper_worker, furi_string_get_cstr(args))) {
                printf("Failed to open file %s\r\n", furi_string_get_cstr(args));
                break;
            }
        } else {
            if(!music_beeper_worker_load_rtttl_from_string(
                   music_beeper_worker, furi_string_get_cstr(args))) {
                printf("Argument is not a file or RTTTL\r\n");
                break;
            }
        }

        printf("Press CTRL+C to stop\r\n");
        music_beeper_worker_set_volume(music_beeper_worker, 1.0f);
        music_beeper_worker_start(music_beeper_worker);
        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(50);
        }
        music_beeper_worker_stop(music_beeper_worker);
    } while(0);

    furi_record_close(RECORD_STORAGE);
    music_beeper_worker_free(music_beeper_worker);
}

void music_beeper_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "music_beeper", CliCommandFlagDefault, music_beeper_cli, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(music_beeper_cli);
#endif
}
