#include "../cli_main_commands.h"
#include <notification/notification_app.h>
#include <math.h>
#include <toolbox/args.h>
#include <toolbox/cli/cli_command.h>

void cli_command_buzzer_print_usage(bool is_freq_subcommand, FuriString* args) {
    if(is_freq_subcommand) {
        cli_print_usage(
            "buzzer freq", "<freq_hz> [<0-...>[<ms|s|m|h>]]", furi_string_get_cstr(args));

    } else {
        cli_print_usage("buzzer note", "<note> [<0-...>[<ms|s|m|h>]]", furi_string_get_cstr(args));
    }
}

// Consider volume effectively zero if below this threshold
#define BUZZER_VOLUME_EPSILON (1e-3f)

float cli_command_buzzer_read_frequency(bool is_freq_subcommand, FuriString* args) {
    float frequency = 0.0f;

    if(is_freq_subcommand) {
        args_read_float_and_trim(args, &frequency);
        return frequency;
    }

    // Extract note frequency from name

    FuriString* note_name_string;
    note_name_string = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, note_name_string)) {
            break;
        }
        const char* note_name = furi_string_get_cstr(note_name_string);
        frequency = notification_messages_notes_frequency_from_name(note_name);
    } while(false);

    furi_string_free(note_name_string);

    return frequency;
}

void cli_command_buzzer_play(
    PipeSide* pipe,
    NotificationApp* notification,
    bool is_freq_subcommand,
    FuriString* args) {
    FuriString* duration_string;
    duration_string = furi_string_alloc();

    do {
        float frequency = cli_command_buzzer_read_frequency(is_freq_subcommand, args);
        if(frequency <= 0.0f) {
            cli_command_buzzer_print_usage(is_freq_subcommand, args);
            break;
        }

        const NotificationMessage notification_buzzer_message = {
            .type = NotificationMessageTypeSoundOn,
            .data.sound.frequency = frequency,
            .data.sound.volume = 1.0,
        };

        // Optional duration
        uint32_t duration_ms = 100;
        if(args_read_string_and_trim(args, duration_string)) {
            if(!args_read_duration(duration_string, &duration_ms, NULL)) {
                cli_command_buzzer_print_usage(is_freq_subcommand, args);
                break;
            }
        }

        const NotificationSequence sound_on_sequence = {
            &notification_buzzer_message,
            &message_do_not_reset,
            NULL,
        };

        // Play sound
        notification_message_block(notification, &sound_on_sequence);

        cli_sleep(pipe, duration_ms);

        // Stop sound
        const NotificationSequence sound_off_sequence = {
            &message_sound_off,
            NULL,
        };
        notification_message_block(notification, &sound_off_sequence);

    } while(false);

    furi_string_free(duration_string);
}

void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    FuriString* command_string;
    command_string = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, command_string)) {
            cli_print_usage("buzzer", "<freq|note>", furi_string_get_cstr(args));
            break;
        }

        // Check volume
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagStealthMode)) {
            printf("Flipper is in stealth mode. Unmute the device to control buzzer.");
            break;
        }
        if(fabsf(notification->settings.speaker_volume) < BUZZER_VOLUME_EPSILON) {
            printf("Sound is disabled in settings. Increase volume to control buzzer.");
            break;
        }

        if(furi_string_cmp(command_string, "freq") == 0) {
            cli_command_buzzer_play(pipe, notification, true, args);
            break;
        } else if(furi_string_cmp(command_string, "note") == 0) {
            cli_command_buzzer_play(pipe, notification, false, args);
            break;
        }

        cli_print_usage("buzzer", "<freq|note>", furi_string_get_cstr(args));

    } while(false);

    furi_string_free(command_string);
    furi_record_close(RECORD_NOTIFICATION);
}

CLI_COMMAND_INTERFACE(buzzer, execute, CliCommandFlagDefault, 2048, CLI_APPID);
