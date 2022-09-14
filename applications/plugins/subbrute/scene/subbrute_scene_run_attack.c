#include "subbrute_scene_run_attack.h"
#include <lib/subghz/transmitter.h>
#include <gui/elements.h>

//uint64_t subbrute_counter = 0;
uint64_t max_value;
bool locked = false;
bool toSave = false;
char subbrute_payload_byte[4];
#define SUBBRUTE_DELAY 1

FuriHalSubGhzPreset str_to_preset(string_t preset) {
    if(string_cmp_str(preset, "FuriHalSubGhzPresetOok270Async") == 0) {
        return FuriHalSubGhzPresetOok270Async;
    }
    if(string_cmp_str(preset, "FuriHalSubGhzPresetOok650Async") == 0) {
        return FuriHalSubGhzPresetOok650Async;
    }
    if(string_cmp_str(preset, "FuriHalSubGhzPreset2FSKDev238Async") == 0) {
        return FuriHalSubGhzPreset2FSKDev238Async;
    }
    if(string_cmp_str(preset, "FuriHalSubGhzPreset2FSKDev476Async") == 0) {
        return FuriHalSubGhzPreset2FSKDev476Async;
    }
    if(string_cmp_str(preset, "FuriHalSubGhzPresetMSK99_97KbAsync") == 0) {
        return FuriHalSubGhzPresetMSK99_97KbAsync;
    }
    if(string_cmp_str(preset, "FuriHalSubGhzPresetMSK99_97KbAsync") == 0) {
        return FuriHalSubGhzPresetMSK99_97KbAsync;
    }
    return FuriHalSubGhzPresetCustom;
}

void subbrute_emit(SubBruteState* context) {
    //FURI_LOG_D(TAG, string_get_cstr(context->flipper_format_string));

    context->transmitter =
        subghz_transmitter_alloc_init(context->environment, string_get_cstr(context->protocol));

    subghz_transmitter_deserialize(context->transmitter, context->flipper_format);
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(str_to_preset(context->preset));

    context->frequency_cal = furi_hal_subghz_set_frequency_and_path(context->frequency);

    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, context->transmitter);
    while(!(furi_hal_subghz_is_async_tx_complete())) {
        furi_delay_ms(1);
    }

    furi_hal_subghz_stop_async_tx();
    subghz_transmitter_stop(context->transmitter);
    furi_hal_subghz_idle();
    subghz_transmitter_free(context->transmitter);
}

void prepare_emit(SubBruteState* context) {
    UNUSED(context);

    furi_hal_subghz_init();
}

void clear_emit(SubBruteState* context) {
    UNUSED(context);

    //furi_hal_subghz_stop_async_tx();
    //furi_hal_subghz_idle();
    furi_hal_subghz_sleep();
}
/*
void subbrute_send_raw_packet(SubBruteState* context) {
    string_reset(context->candidate);

    // Payload to padded binary string
    int* binaryNum = (int*)malloc(sizeof(int) * context->bit);
    uint32_t i = 0;
    for(i = 0; i < context->bit; i++) {
        binaryNum[i] = 0;
    }
    i = 0;
    uint64_t counter = context->payload;
    while(counter > 0) {
        binaryNum[i] = counter % 2;
        counter = counter / 2;
        i++;
    }

    // printing binary array in reverse order and build raw payload
    for(uint32_t loop = 0; loop < context->repeat; loop++) {
        for(int j = (int)context->bit - 1; j >= 0; j--) {
            if(binaryNum[j] == 1) {
                string_cat(context->candidate, context->subbrute_raw_one);
            } else {
                string_cat(context->candidate, context->subbrute_raw_zero);
            }
        }
        string_cat(context->candidate, context->subbrute_raw_stop);
    }

    free(binaryNum);

    string_init_printf(
        context->flipper_format_string,
        "Filetype: Flipper SubGhz RAW File\n"
        "Version: 1\n"
        "Frequency: %d\n"
        "Preset: %s\n"
        "Protocol: RAW\n"
        "RAW_Data: %s",
        context->frequency,
        string_get_cstr(context->preset),
        string_get_cstr(context->candidate));

    subbrute_emit(context);
}
*/
void subbrute_send_packet_parsed(SubBruteState* context) {
    if(context->attack == SubBruteAttackLoadFile) {
        snprintf(subbrute_payload_byte, 4, "%02X ", (uint8_t)context->payload);
        string_replace_at(context->candidate, context->str_index, 3, subbrute_payload_byte);
    } else {
        string_t buffer;
        string_init(buffer);
        string_init_printf(buffer, "%16X", context->payload);
        int j = 0;
        string_set_str(context->candidate, "                       ");
        for(uint8_t i = 0; i < 16; i++) {
            if(string_get_char(buffer, i) != ' ') {
                string_set_char(context->candidate, i + j, string_get_char(buffer, i));
            } else {
                string_set_char(context->candidate, i + j, '0');
            }
            if(i % 2 != 0) {
                j++;
            }
        }
        string_clear(buffer);
    }
    if(strcmp(string_get_cstr(context->protocol), "Princeton") == 0) {
        string_init_printf(
            context->flipper_format_string,
            "Filetype: Flipper SubGhz Key File\n"
            "Version: 1\n"
            "Frequency: %u\n"
            "Preset: %s\n"
            "Protocol: %s\n"
            "Bit: %d\n"
            "Key: %s\n"
            "TE: %d\n",
            context->frequency,
            string_get_cstr(context->preset),
            string_get_cstr(context->protocol),
            context->bit,
            string_get_cstr(context->candidate),
            context->te);
    } else {
        string_init_printf(
            context->flipper_format_string,
            "Filetype: Flipper SubGhz Key File\n"
            "Version: 1\n"
            "Frequency: %u\n"
            "Preset: %s\n"
            "Protocol: %s\n"
            "Bit: %d\n"
            "Key: %s\n",
            context->frequency,
            string_get_cstr(context->preset),
            string_get_cstr(context->protocol),
            context->bit,
            string_get_cstr(context->candidate));
    }

    stream_clean(context->stream);
    stream_write_string(context->stream, context->flipper_format_string);
}

void subbrute_send_packet(SubBruteState* context) {
    ///if(string_cmp_str(context->protocol, "RAW") == 0) {
    //   subbrute_send_raw_packet(context);
    //} else {
    subbrute_send_packet_parsed(context);
    subbrute_emit(context);
    //}
    string_clear(context->flipper_format_string);
}

void subbrute_scene_run_attack_on_exit(SubBruteState* context) {
    if(!toSave) {
        clear_emit(context);
        furi_thread_free(context->bruthread);
        flipper_format_free(context->flipper_format);
        subghz_receiver_free(context->receiver);
        subghz_environment_free(context->environment);
    }
}

void subbrute_scene_run_attack_on_tick(SubBruteState* context) {
    if(!context->is_attacking || locked) {
        return;
    }
    //if(0 != subbrute_counter) {
    locked = true;
    subbrute_send_packet(context);

    if(context->payload == max_value) {
        //context->payload = 0x00;
        //subbrute_counter = 0;
        context->is_attacking = false;
        notification_message(context->notify, &sequence_blink_stop);
        notification_message(context->notify, &sequence_single_vibro);
    } else {
        context->payload++;
    }
    locked = false;
    //}
    /*if(subbrute_counter > SUBBRUTE_DELAY) {
        subbrute_counter = 0;
    } else {
        subbrute_counter++;
    }*/
}
void subbrute_run_timer(SubBruteState* context) {
    while(true) {
        if(!context->is_attacking) {
            context->is_thread_running = false;
            break;
        }
        //furi_delay_ms(10);
        subbrute_scene_run_attack_on_tick(context);
    }
}

// entrypoint for worker
static int32_t subbrute_worker_thread(void* ctx) {
    SubBruteState* app = ctx;
    subbrute_run_timer(app);
    return 0;
}

void start_bruthread(SubBruteState* app) {
    if(!app->is_thread_running) {
        furi_thread_start(app->bruthread);
        app->is_thread_running = true;
    }
}

void subbrute_scene_run_attack_on_enter(SubBruteState* context) {
    if(!toSave) {
        if(context->attack == SubBruteAttackLoadFile) {
            max_value = 0xFF;
        } else {
            string_t max_value_s;
            string_init(max_value_s);
            for(uint8_t i = 0; i < context->bit; i++) {
                string_cat_printf(max_value_s, "1");
            }
            max_value = (uint64_t)strtol(string_get_cstr(max_value_s), NULL, 2);
            string_clear(max_value_s);
        }
        context->str_index = (context->key_index * 3);
        string_init_set(context->candidate, context->key);
        context->flipper_format = flipper_format_string_alloc();
        context->stream = flipper_format_get_raw_stream(context->flipper_format);
        context->environment = subghz_environment_alloc();
        context->receiver = subghz_receiver_alloc_init(context->environment);
        subghz_receiver_set_filter(context->receiver, SubGhzProtocolFlag_Decodable);

        prepare_emit(context);
        context->bruthread = furi_thread_alloc();
        furi_thread_set_name(context->bruthread, "SubBrute Worker");
        furi_thread_set_stack_size(context->bruthread, 2048);
        furi_thread_set_context(context->bruthread, context);
        furi_thread_set_callback(context->bruthread, subbrute_worker_thread);
    } else {
        toSave = false;
    }
}

void subbrute_scene_run_attack_on_event(SubBruteEvent event, SubBruteState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
                break;
            case InputKeyUp:
                if(!context->is_attacking) {
                    subbrute_send_packet_parsed(context);
                    string_clear(context->flipper_format_string);
                    toSave = true;
                    context->current_scene = SceneSaveName;
                }
                break;
            case InputKeyLeft:
                if(!context->is_attacking && context->payload > 0x00) {
                    context->payload--;
                    subbrute_send_packet(context);
                    notification_message(context->notify, &sequence_blink_blue_10);
                } else if(!context->is_attacking && context->payload == 0x00) {
                    context->payload = max_value;
                    subbrute_send_packet(context);
                    notification_message(context->notify, &sequence_blink_blue_10);
                }
                break;
            case InputKeyRight:
                if(!context->is_attacking && context->payload < max_value) {
                    context->payload++;
                    subbrute_send_packet(context);
                    notification_message(context->notify, &sequence_blink_blue_10);
                } else if(!context->is_attacking && context->payload == max_value) {
                    context->payload = 0x00;
                    subbrute_send_packet(context);
                    notification_message(context->notify, &sequence_blink_blue_10);
                }
                break;
            case InputKeyOk:
                if(!context->is_attacking) {
                    if(context->payload == max_value) {
                        context->payload = 0x00;
                        //subbrute_counter = 0;
                    }
                    context->is_attacking = true;
                    start_bruthread(context);
                    notification_message(context->notify, &sequence_blink_start_blue);
                } else {
                    context->is_attacking = false;
                    //context->close_thread_please = true;
                    if(context->is_thread_running && context->bruthread) {
                        furi_thread_join(context->bruthread); // wait until thread is finished
                    }
                    //context->close_thread_please = false;
                    notification_message(context->notify, &sequence_blink_stop);
                    notification_message(context->notify, &sequence_single_vibro);
                }
                break;
            case InputKeyBack:
                locked = false;
                //context->close_thread_please = true;
                context->is_attacking = false;
                if(context->is_thread_running && context->bruthread) {
                    furi_thread_join(context->bruthread); // wait until thread is finished
                }
                //context->close_thread_please = false;
                string_reset(context->notification_msg);
                context->payload = 0x00;
                //subbrute_counter = 0;
                notification_message(context->notify, &sequence_blink_stop);
                if(context->attack == SubBruteAttackLoadFile) {
                    context->current_scene = SceneSelectField;
                } else {
                    context->current_scene = SceneEntryPoint;
                }
                break;
            }
        }
    }
}

void subbrute_scene_run_attack_on_draw(Canvas* canvas, SubBruteState* context) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    //canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignTop, "Fire in the hole!");

    char msg_index[26];
    snprintf(
        msg_index, sizeof(msg_index), "< %04d / %04d >", (int)context->payload, (int)max_value);

    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignTop, msg_index);

    canvas_set_font(canvas, FontSecondary);
    char start_stop_msg[20];
    snprintf(start_stop_msg, sizeof(start_stop_msg), " Press (^) to save ");
    if(context->is_attacking) {
        elements_button_center(canvas, "Stop");
    } else {
        elements_button_center(canvas, "Start");
    }
    canvas_draw_str_aligned(canvas, 64, 39, AlignCenter, AlignTop, start_stop_msg);
}
