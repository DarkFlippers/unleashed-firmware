#include <furi.h>
#include <notification/notification.h>
#include <music_worker/music_worker.h>
#include <cli/cli.h>
#include <toolbox/args.h>

#define TAG "SpeakerDebug"
#define CLI_COMMAND "speaker_debug"

typedef enum {
    SpeakerDebugAppMessageTypeStop,
} SpeakerDebugAppMessageType;

typedef struct {
    SpeakerDebugAppMessageType type;
} SpeakerDebugAppMessage;

typedef struct {
    MusicWorker* music_worker;
    FuriMessageQueue* message_queue;
    Cli* cli;
} SpeakerDebugApp;

static SpeakerDebugApp* speaker_app_alloc() {
    SpeakerDebugApp* app = (SpeakerDebugApp*)malloc(sizeof(SpeakerDebugApp));
    app->music_worker = music_worker_alloc();
    app->message_queue = furi_message_queue_alloc(8, sizeof(SpeakerDebugAppMessage));
    app->cli = furi_record_open(RECORD_CLI);
    return app;
}

static void speaker_app_free(SpeakerDebugApp* app) {
    music_worker_free(app->music_worker);
    furi_message_queue_free(app->message_queue);
    furi_record_close(RECORD_CLI);
    free(app);
}

static void speaker_app_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);

    SpeakerDebugApp* app = (SpeakerDebugApp*)context;
    SpeakerDebugAppMessage message;
    FuriString* cmd = furi_string_alloc();

    if(!args_read_string_and_trim(args, cmd)) {
        furi_string_free(cmd);
        printf("Usage:\r\n");
        printf("\t" CLI_COMMAND " stop\r\n");
        return;
    }

    if(furi_string_cmp(cmd, "stop") == 0) {
        message.type = SpeakerDebugAppMessageTypeStop;
        FuriStatus status = furi_message_queue_put(app->message_queue, &message, 100);
        if(status != FuriStatusOk) {
            printf("Failed to send message\r\n");
        } else {
            printf("Stopping\r\n");
        }
    } else {
        printf("Usage:\r\n");
        printf("\t" CLI_COMMAND " stop\r\n");
    }

    furi_string_free(cmd);
}

static bool speaker_app_music_play(SpeakerDebugApp* app, const char* rtttl) {
    if(music_worker_is_playing(app->music_worker)) {
        music_worker_stop(app->music_worker);
    }

    if(!music_worker_load_rtttl_from_string(app->music_worker, rtttl)) {
        FURI_LOG_E(TAG, "Failed to load RTTTL");
        return false;
    }

    music_worker_set_volume(app->music_worker, 1.0f);
    music_worker_start(app->music_worker);

    return true;
}

static void speaker_app_music_stop(SpeakerDebugApp* app) {
    if(music_worker_is_playing(app->music_worker)) {
        music_worker_stop(app->music_worker);
    }
}

static void speaker_app_run(SpeakerDebugApp* app, const char* arg) {
    if(!arg || !speaker_app_music_play(app, arg)) {
        FURI_LOG_E(TAG, "Provided RTTTL is invalid");
        return;
    }

    cli_add_command(app->cli, CLI_COMMAND, CliCommandFlagParallelSafe, speaker_app_cli, app);

    SpeakerDebugAppMessage message;
    FuriStatus status;
    while(true) {
        status = furi_message_queue_get(app->message_queue, &message, FuriWaitForever);

        if(status == FuriStatusOk) {
            if(message.type == SpeakerDebugAppMessageTypeStop) {
                speaker_app_music_stop(app);
                break;
            }
        }
    }

    cli_delete_command(app->cli, CLI_COMMAND);
}

int32_t speaker_debug_app(void* arg) {
    SpeakerDebugApp* app = speaker_app_alloc();
    speaker_app_run(app, arg);
    speaker_app_free(app);
    return 0;
}
