#include "flipfrid_scene_run_attack.h"
#include <gui/elements.h>

uint8_t counter = 0;

uint8_t id_list[17][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Null bytes
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Only FF
    {0x11, 0x11, 0x11, 0x11, 0x11}, // Only 11
    {0x22, 0x22, 0x22, 0x22, 0x22}, // Only 22
    {0x33, 0x33, 0x33, 0x33, 0x33}, // Only 33
    {0x44, 0x44, 0x44, 0x44, 0x44}, // Only 44
    {0x55, 0x55, 0x55, 0x55, 0x55}, // Only 55
    {0x66, 0x66, 0x66, 0x66, 0x66}, // Only 66
    {0x77, 0x77, 0x77, 0x77, 0x77}, // Only 77
    {0x88, 0x88, 0x88, 0x88, 0x88}, // Only 88
    {0x99, 0x99, 0x99, 0x99, 0x99}, // Only 99
    {0x12, 0x34, 0x56, 0x78, 0x9A}, // Incremental UID
    {0x9A, 0x78, 0x56, 0x34, 0x12}, // Decremental UID
    {0x04, 0xd0, 0x9b, 0x0d, 0x6a}, // From arha
    {0x34, 0x00, 0x29, 0x3d, 0x9e}, // From arha
    {0x04, 0xdf, 0x00, 0x00, 0x01}, // From arha
    {0xCA, 0xCA, 0xCA, 0xCA, 0xCA}, // From arha
};

uint8_t id_list_hid[14][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Null bytes
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Only FF
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11}, // Only 11
    {0x22, 0x22, 0x22, 0x22, 0x22, 0x22}, // Only 22
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33}, // Only 33
    {0x44, 0x44, 0x44, 0x44, 0x44, 0x44}, // Only 44
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55}, // Only 55
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66}, // Only 66
    {0x77, 0x77, 0x77, 0x77, 0x77, 0x77}, // Only 77
    {0x88, 0x88, 0x88, 0x88, 0x88, 0x88}, // Only 88
    {0x99, 0x99, 0x99, 0x99, 0x99, 0x99}, // Only 99
    {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}, // Incremental UID
    {0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12}, // Decremental UID
    {0xCA, 0xCA, 0xCA, 0xCA, 0xCA, 0xCA}, // From arha
};

uint8_t id_list_pac[17][4] = {
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
    {0x04, 0xd0, 0x9b, 0x0d}, // From arha
    {0x34, 0x00, 0x29, 0x3d}, // From arha
    {0x04, 0xdf, 0x00, 0x00}, // From arha
    {0xCA, 0xCA, 0xCA, 0xCA}, // From arha
};

uint8_t id_list_h[14][3] = {
    {0x00, 0x00, 0x00}, // Null bytes
    {0xFF, 0xFF, 0xFF}, // Only FF
    {0x11, 0x11, 0x11}, // Only 11
    {0x22, 0x22, 0x22}, // Only 22
    {0x33, 0x33, 0x33}, // Only 33
    {0x44, 0x44, 0x44}, // Only 44
    {0x55, 0x55, 0x55}, // Only 55
    {0x66, 0x66, 0x66}, // Only 66
    {0x77, 0x77, 0x77}, // Only 77
    {0x88, 0x88, 0x88}, // Only 88
    {0x99, 0x99, 0x99}, // Only 99
    {0x12, 0x34, 0x56}, // Incremental UID
    {0x56, 0x34, 0x12}, // Decremental UID
    {0xCA, 0xCA, 0xCA}, // From arha
};

void flipfrid_scene_run_attack_on_enter(FlipFridState* context) {
    context->time_between_cards = 10;
    context->attack_step = 0;
    context->attack_stop_called = false;
    context->dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    context->worker = lfrfid_worker_alloc(context->dict);
    if(context->proto == HIDProx) {
        context->protocol = protocol_dict_get_protocol_by_name(context->dict, "HIDProx");
    } else if(context->proto == PAC) {
        context->protocol = protocol_dict_get_protocol_by_name(context->dict, "PAC/Stanley");
    } else if(context->proto == H10301) {
        context->protocol = protocol_dict_get_protocol_by_name(context->dict, "H10301");
    } else {
        context->protocol = protocol_dict_get_protocol_by_name(context->dict, "EM4100");
    }
}

void flipfrid_scene_run_attack_on_exit(FlipFridState* context) {
    if(context->workr_rund) {
        lfrfid_worker_stop(context->worker);
        lfrfid_worker_stop_thread(context->worker);
        context->workr_rund = false;
    }
    lfrfid_worker_free(context->worker);
    protocol_dict_free(context->dict);
    notification_message(context->notify, &sequence_blink_stop);
}

void flipfrid_scene_run_attack_on_tick(FlipFridState* context) {
    if(context->is_attacking) {
        if(1 == counter) {
            protocol_dict_set_data(context->dict, context->protocol, context->payload, 6);
            lfrfid_worker_free(context->worker);
            context->worker = lfrfid_worker_alloc(context->dict);
            lfrfid_worker_start_thread(context->worker);
            lfrfid_worker_emulate_start(context->worker, context->protocol);
            context->workr_rund = true;
        } else if(0 == counter) {
            if(context->workr_rund) {
                lfrfid_worker_stop(context->worker);
                lfrfid_worker_stop_thread(context->worker);
                context->workr_rund = false;
                furi_delay_ms(200);
            }
            switch(context->attack) {
            case FlipFridAttackDefaultValues:
                if(context->proto == EM4100) {
                    context->payload[0] = id_list[context->attack_step][0];
                    context->payload[1] = id_list[context->attack_step][1];
                    context->payload[2] = id_list[context->attack_step][2];
                    context->payload[3] = id_list[context->attack_step][3];
                    context->payload[4] = id_list[context->attack_step][4];

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
                } else if(context->proto == PAC) {
                    context->payload[0] = id_list_pac[context->attack_step][0];
                    context->payload[1] = id_list_pac[context->attack_step][1];
                    context->payload[2] = id_list_pac[context->attack_step][2];
                    context->payload[3] = id_list_pac[context->attack_step][3];

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
                } else if(context->proto == H10301) {
                    context->payload[0] = id_list_h[context->attack_step][0];
                    context->payload[1] = id_list_h[context->attack_step][1];
                    context->payload[2] = id_list_h[context->attack_step][2];

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
                } else {
                    context->payload[0] = id_list_hid[context->attack_step][0];
                    context->payload[1] = id_list_hid[context->attack_step][1];
                    context->payload[2] = id_list_hid[context->attack_step][2];
                    context->payload[3] = id_list_hid[context->attack_step][3];
                    context->payload[4] = id_list_hid[context->attack_step][4];
                    context->payload[5] = id_list_hid[context->attack_step][5];

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

            case FlipFridAttackBfCustomerId:
                if(context->proto == EM4100) {
                    context->payload[0] = context->attack_step;
                    context->payload[1] = 0x00;
                    context->payload[2] = 0x00;
                    context->payload[3] = 0x00;
                    context->payload[4] = 0x00;

                    if(context->attack_step == 255) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                    } else {
                        context->attack_step++;
                    }
                    break;
                } else if(context->proto == PAC) {
                    context->payload[0] = context->attack_step;
                    context->payload[1] = 0x00;
                    context->payload[2] = 0x00;
                    context->payload[3] = 0x00;

                    if(context->attack_step == 255) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_single_vibro);
                    } else {
                        context->attack_step++;
                    }
                    break;
                } else if(context->proto == H10301) {
                    context->payload[0] = context->attack_step;
                    context->payload[1] = 0x00;
                    context->payload[2] = 0x00;

                    if(context->attack_step == 255) {
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
                    context->payload[0] = context->attack_step;
                    context->payload[1] = 0x00;
                    context->payload[2] = 0x00;
                    context->payload[3] = 0x00;
                    context->payload[4] = 0x00;
                    context->payload[5] = 0x00;

                    if(context->attack_step == 255) {
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

            case FlipFridAttackLoadFile:
                if(context->proto == EM4100) {
                    context->payload[0] = context->data[0];
                    context->payload[1] = context->data[1];
                    context->payload[2] = context->data[2];
                    context->payload[3] = context->data[3];
                    context->payload[4] = context->data[4];

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
                } else if(context->proto == PAC) {
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
                } else if(context->proto == H10301) {
                    context->payload[0] = context->data[0];
                    context->payload[1] = context->data[1];
                    context->payload[2] = context->data[2];

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
                    context->payload[4] = context->data[4];
                    context->payload[5] = context->data[5];

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

            case FlipFridAttackLoadFileCustomUids:
                if(context->proto == EM4100) {
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
                        if(furi_string_size(context->data_str) != 11) break;
                        break;
                    }
                    if(end_of_list) break;
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
                    if(furi_string_size(context->data_str) != 11) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_error);
                        break;
                    };

                    // string is valid, parse it in context->payload
                    for(uint8_t i = 0; i < 5; i++) {
                        char temp_str[3];
                        temp_str[0] = furi_string_get_cstr(context->data_str)[i * 2];
                        temp_str[1] = furi_string_get_cstr(context->data_str)[i * 2 + 1];
                        temp_str[2] = '\0';
                        context->payload[i] = (uint8_t)strtol(temp_str, NULL, 16);
                    }
                    break;
                } else if(context->proto == PAC) {
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
                    if(end_of_list) break;
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
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
                } else if(context->proto == H10301) {
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
                        if(furi_string_size(context->data_str) != 7) break;
                        break;
                    }
                    if(end_of_list) break;
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
                    if(furi_string_size(context->data_str) != 7) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_error);
                        break;
                    };

                    // string is valid, parse it in context->payload
                    for(uint8_t i = 0; i < 3; i++) {
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
                        if(furi_string_size(context->data_str) != 13) break;
                        break;
                    }
                    FURI_LOG_D(TAG, furi_string_get_cstr(context->data_str));
                    if(end_of_list) break;
                    if(furi_string_size(context->data_str) != 13) {
                        context->attack_step = 0;
                        counter = 0;
                        context->is_attacking = false;
                        notification_message(context->notify, &sequence_blink_stop);
                        notification_message(context->notify, &sequence_error);
                        break;
                    };

                    // string is valid, parse it in context->payload
                    for(uint8_t i = 0; i < 6; i++) {
                        char temp_str[3];
                        temp_str[0] = furi_string_get_cstr(context->data_str)[i * 2];
                        temp_str[1] = furi_string_get_cstr(context->data_str)[i * 2 + 1];
                        temp_str[2] = '\0';
                        context->payload[i] = (uint8_t)strtol(temp_str, NULL, 16);
                    }
                    break;
                default:
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

void flipfrid_scene_run_attack_on_event(FlipFridEvent event, FlipFridState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
                break;
            case InputKeyUp:
                break;
            case InputKeyLeft:
                if(!context->is_attacking) {
                    if(context->time_between_cards > 5) {
                        context->time_between_cards--;
                    }
                }
                break;
            case InputKeyRight:
                if(!context->is_attacking) {
                    if(context->time_between_cards < 70) {
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
                    if(context->attack == FlipFridAttackLoadFileCustomUids) {
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
                    if(context->time_between_cards > 0) {
                        if((context->time_between_cards - 10) > 5) {
                            context->time_between_cards -= 10;
                        }
                    }
                }
                break;
            case InputKeyRight:
                if(!context->is_attacking) {
                    if(context->time_between_cards < 70) {
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

void flipfrid_scene_run_attack_on_draw(Canvas* canvas, FlipFridState* context) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    //canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 2, AlignCenter, AlignTop, furi_string_get_cstr(context->attack_name));

    char uid[18];
    char speed[16];
    if(context->proto == HIDProx) {
        snprintf(
            uid,
            sizeof(uid),
            "%02X:%02X:%02X:%02X:%02X:%02X",
            context->payload[0],
            context->payload[1],
            context->payload[2],
            context->payload[3],
            context->payload[4],
            context->payload[5]);
    } else if(context->proto == PAC) {
        snprintf(
            uid,
            sizeof(uid),
            "%02X:%02X:%02X:%02X",
            context->payload[0],
            context->payload[1],
            context->payload[2],
            context->payload[3]);
    } else if(context->proto == H10301) {
        snprintf(
            uid,
            sizeof(uid),
            "%02X:%02X:%02X",
            context->payload[0],
            context->payload[1],
            context->payload[2]);
    } else {
        snprintf(
            uid,
            sizeof(uid),
            "%02X:%02X:%02X:%02X:%02X",
            context->payload[0],
            context->payload[1],
            context->payload[2],
            context->payload[3],
            context->payload[4]);
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
