#include "subghz_receiver.h"
#include "../subghz_i.h"
#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>
#include <lib/subghz/protocols/subghz_protocol_princeton.h>

#include <assets_icons.h>

#define FRAME_HEIGHT 12
#define MAX_LEN_PX 100
#define MENU_ITEMS 4

#define COUNT_FREQUNCY_HOPPER 3
const uint32_t subghz_frequencies_hopper[] = {
    /* 300 - 348 */
    315000000,
    /* 387 - 464 */
    433920000, /* LPD433 mid */
    /* 779 - 928 */
    868350000,
};

typedef enum {
    ReceiverSceneStart,
    ReceiverSceneMain,
    ReceiverSceneConfig,
    ReceiverSceneInfo,
} SubghzReceiverScene;

typedef enum {
    SubGhzHopperStateOFF,
    SubGhzHopperStatePause,
    SubGhzHopperStateRunnig,
    SubGhzHopperStateRSSITimeOut,
} SubGhzHopperState;

static const Icon* ReceiverItemIcons[] = {
    [TYPE_PROTOCOL_UNKNOWN] = &I_Quest_7x8,
    [TYPE_PROTOCOL_STATIC] = &I_Unlock_7x8,
    [TYPE_PROTOCOL_DYNAMIC] = &I_Lock_7x8,
};

struct SubghzReceiver {
    View* view;
    SubghzReceiverCallback callback;
    void* context;
    SubGhzWorker* worker;
    SubGhzProtocol* protocol;
    osTimerId timer;
    SubGhzHopperState hopper_state;
    uint8_t hopper_timeout;
    uint32_t event_key_sequence;
};

typedef struct {
    string_t text;
    uint16_t scene;
    SubGhzProtocolCommon* protocol_result;
    SubGhzHistory* history;
    uint8_t frequency;
    uint8_t temp_frequency;
    uint32_t real_frequency;

    uint16_t idx;
    uint16_t list_offset;
    uint16_t history_item;
    bool menu;
} SubghzReceiverModel;

void subghz_receiver_set_callback(
    SubghzReceiver* subghz_receiver,
    SubghzReceiverCallback callback,
    void* context) {
    furi_assert(subghz_receiver);
    furi_assert(callback);
    subghz_receiver->callback = callback;
    subghz_receiver->context = context;
}

void subghz_receiver_set_protocol(
    SubghzReceiver* subghz_receiver,
    SubGhzProtocolCommon* protocol_result,
    SubGhzProtocol* protocol) {
    furi_assert(subghz_receiver);
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            model->protocol_result = protocol_result;
            return true;
        });
    subghz_receiver->protocol = protocol;
}

SubGhzProtocolCommon* subghz_receiver_get_protocol(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    SubGhzProtocolCommon* result = NULL;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            result = model->protocol_result;
            return false;
        });
    return result;
}

void subghz_receiver_set_worker(SubghzReceiver* subghz_receiver, SubGhzWorker* worker) {
    furi_assert(subghz_receiver);
    subghz_receiver->worker = worker;
}

static void subghz_receiver_update_offset(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            size_t history_item = model->history_item;
            uint16_t bounds = history_item > 3 ? 2 : history_item;

            if(history_item > 3 && model->idx >= history_item - 1) {
                model->list_offset = model->idx - 3;
            } else if(model->list_offset < model->idx - bounds) {
                model->list_offset = CLAMP(model->list_offset + 1, history_item - bounds, 0);
            } else if(model->list_offset > model->idx - bounds) {
                model->list_offset = CLAMP(model->idx - 1, history_item - bounds, 0);
            }
            return true;
        });
}

static void subghz_receiver_draw_frame(Canvas* canvas, uint16_t idx, bool scrollbar) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0 + idx * FRAME_HEIGHT, scrollbar ? 122 : 127, FRAME_HEIGHT);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_dot(canvas, 0, 0 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 1, 0 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, 0, (0 + idx * FRAME_HEIGHT) + 1);

    canvas_draw_dot(canvas, 0, (0 + idx * FRAME_HEIGHT) + 11);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, 0 + idx * FRAME_HEIGHT);
    canvas_draw_dot(canvas, scrollbar ? 121 : 126, (0 + idx * FRAME_HEIGHT) + 11);
}

void subghz_receiver_draw(Canvas* canvas, SubghzReceiverModel* model) {
    bool scrollbar = model->history_item > 4;
    string_t str_buff;
    char buffer[64];
    uint32_t frequency;
    string_init(str_buff);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    switch(model->scene) {
    case ReceiverSceneMain:
        for(size_t i = 0; i < MIN(model->history_item, MENU_ITEMS); ++i) {
            size_t idx = CLAMP(i + model->list_offset, model->history_item, 0);
            subghz_history_get_text_item_menu(model->history, str_buff, idx);
            elements_string_fit_width(canvas, str_buff, scrollbar ? MAX_LEN_PX - 6 : MAX_LEN_PX);
            if(model->idx == idx) {
                subghz_receiver_draw_frame(canvas, i, scrollbar);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }
            canvas_draw_icon(
                canvas,
                1,
                2 + i * FRAME_HEIGHT,
                ReceiverItemIcons[subghz_history_get_type_protocol(model->history, idx)]);
            canvas_draw_str(canvas, 15, 9 + i * FRAME_HEIGHT, string_get_cstr(str_buff));
            string_clean(str_buff);
        }
        if(scrollbar) {
            elements_scrollbar_pos(canvas, 128, 0, 49, model->idx, model->history_item);
        }
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);

        elements_button_left(canvas, "Config");
        canvas_draw_line(canvas, 46, 51, 125, 51);
        if(subghz_history_get_text_space_left(model->history, str_buff)) {
            canvas_draw_str(canvas, 54, 62, string_get_cstr(str_buff));
        } else {
            if((model->real_frequency / 1000 % 10) > 4) {
                frequency = model->real_frequency + 10000;
            } else {
                frequency = model->real_frequency;
            }
            snprintf(
                buffer,
                sizeof(buffer),
                "%03ld.%02ld",
                frequency / 1000000 % 1000,
                frequency / 10000 % 100);
            canvas_draw_str(canvas, 44, 62, buffer);
            canvas_draw_str(canvas, 79, 62, "AM");
            canvas_draw_str(canvas, 96, 62, string_get_cstr(str_buff));
        }
        break;

    case ReceiverSceneStart:
        canvas_draw_icon(canvas, 0, 0, &I_Scanning_123x52);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 63, 46, "Scanning...");
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "Config");
        if((model->real_frequency / 1000 % 10) > 4) {
            frequency = model->real_frequency + 10000;
        } else {
            frequency = model->real_frequency;
        }
        snprintf(
            buffer,
            sizeof(buffer),
            "%03ld.%02ld",
            frequency / 1000000 % 1000,
            frequency / 10000 % 100);
        canvas_draw_str(canvas, 44, 62, buffer);
        canvas_draw_str(canvas, 79, 62, "AM");
        subghz_history_get_text_space_left(model->history, str_buff);
        canvas_draw_str(canvas, 96, 62, string_get_cstr(str_buff));
        canvas_draw_line(canvas, 46, 51, 125, 51);
        break;

    case ReceiverSceneConfig:
        if(model->frequency < subghz_frequencies_count) {
            snprintf(
                buffer,
                sizeof(buffer),
                "Frequency:  < %03ld.%03ldMHz >",
                model->real_frequency / 1000000 % 1000,
                model->real_frequency / 1000 % 1000);
            canvas_draw_str(canvas, 0, 8, buffer);
            canvas_draw_str(canvas, 0, 18, "Frequency Hopping: <OFF>");
        } else {
            canvas_draw_str(canvas, 0, 8, "Frequency: < --- >");
            canvas_draw_str(canvas, 0, 18, "Frequency Hopping: <ON>");
        }
        canvas_draw_str(canvas, 0, 28, "Modulation: <AM>");

        elements_button_center(canvas, "Save");
        break;

    case ReceiverSceneInfo:
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text(canvas, 0, 8, string_get_cstr(model->text));
        snprintf(
            buffer,
            sizeof(buffer),
            "%03ld.%03ld",
            subghz_history_get_frequency(model->history, model->idx) / 1000000 % 1000,
            subghz_history_get_frequency(model->history, model->idx) / 1000 % 1000);
        canvas_draw_str(canvas, 90, 8, buffer);
        if(model->protocol_result && model->protocol_result->to_save_string &&
           strcmp(model->protocol_result->name, "KeeLoq")) {
            elements_button_right(canvas, "Save");
            elements_button_center(canvas, "Send");
        }
        break;

    default:

        break;
    }

    string_clear(str_buff);
}

void subghz_receiver_history_full(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;
    subghz_receiver->callback(SubghzReceverEventSendHistoryFull, subghz_receiver->context);
    subghz_receiver->hopper_state = SubGhzHopperStateOFF;
}

bool subghz_receiver_input(InputEvent* event, void* context) {
    furi_assert(context);

    uint8_t scene = 0;
    SubghzReceiver* subghz_receiver = context;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            scene = model->scene;
            return false;
        });

    bool can_be_saved = false;

    switch(scene) {
    case ReceiverSceneMain:
        if(event->key == InputKeyBack && event->type == InputTypeShort) {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    model->idx = 0;
                    model->list_offset = 0;
                    model->history_item = 0;
                    subghz_history_clean(model->history);
                    return true;
                });
            return false;
        } else if(
            event->key == InputKeyUp &&
            (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    if(model->idx != 0) model->idx--;
                    return true;
                });
        } else if(
            event->key == InputKeyDown &&
            (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    if(model->idx != subghz_history_get_item(model->history) - 1) model->idx++;
                    return true;
                });
        } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
            subghz_receiver->hopper_state = SubGhzHopperStatePause;
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    model->scene = ReceiverSceneConfig;
                    model->temp_frequency = model->frequency;
                    return true;
                });
            subghz_receiver->callback(SubghzReceverEventConfig, subghz_receiver->context);
        } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            subghz_receiver->event_key_sequence = event->sequence;
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    string_clean(model->text);
                    model->protocol_result = subghz_protocol_get_by_name(
                        subghz_receiver->protocol,
                        subghz_history_get_name(model->history, model->idx));
                    if(model->protocol_result->to_load_protocol != NULL) {
                        model->protocol_result->to_load_protocol(
                            model->protocol_result,
                            subghz_history_get_raw_data(model->history, model->idx));
                        model->protocol_result->to_string(model->protocol_result, model->text);
                        model->scene = ReceiverSceneInfo;
                    }
                    return true;
                });
        }
        break;

    case ReceiverSceneInfo:
        with_view_model(
            subghz_receiver->view, (SubghzReceiverModel * model) {
                can_be_saved =
                    (model->protocol_result && model->protocol_result->to_save_string &&
                     strcmp(model->protocol_result->name, "KeeLoq"));
                return false;
            });
        if(event->key == InputKeyBack && event->type == InputTypeShort) {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    subghz_rx_end(subghz_receiver->worker);
                    model->real_frequency =
                        subghz_rx(subghz_receiver->worker, subghz_frequencies[model->frequency]);
                    subghz_receiver->hopper_state = SubGhzHopperStateRunnig;
                    model->scene = ReceiverSceneMain;
                    return true;
                });
            subghz_receiver->callback(SubghzReceverEventMain, subghz_receiver->context);
        } else if(can_be_saved && event->key == InputKeyRight) {
            subghz_receiver->callback(SubghzReceverEventSave, subghz_receiver->context);
            return false;
        } else if(
            can_be_saved && event->key == InputKeyOk && event->type == InputTypePress &&
            subghz_receiver->event_key_sequence != event->sequence) {
            subghz_receiver->hopper_state = SubGhzHopperStatePause;
            subghz_rx_end(subghz_receiver->worker);
            subghz_receiver->callback(SubghzReceverEventSendStart, subghz_receiver->context);
            return true;
        } else if(
            can_be_saved && event->key == InputKeyOk && event->type == InputTypeRelease &&
            subghz_receiver->event_key_sequence != event->sequence) {
            subghz_receiver->callback(SubghzReceverEventSendStop, subghz_receiver->context);
            return true;
        }
        break;

    case ReceiverSceneConfig:
        if(event->type != InputTypeShort) return false;
        if(event->key == InputKeyBack) {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    model->frequency = model->temp_frequency;
                    model->real_frequency = subghz_frequencies[model->frequency];
                    subghz_receiver->hopper_state = SubGhzHopperStateRunnig;
                    if(subghz_history_get_item(model->history) == 0) {
                        model->scene = ReceiverSceneStart;
                    } else {
                        model->scene = ReceiverSceneMain;
                    }
                    return true;
                });
            subghz_receiver->callback(SubghzReceverEventMain, subghz_receiver->context);
        } else if(event->key == InputKeyOk) {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    if(model->frequency < subghz_frequencies_count) {
                        subghz_rx_end(subghz_receiver->worker);
                        model->real_frequency = subghz_rx(
                            subghz_receiver->worker, subghz_frequencies[model->frequency]);
                        subghz_receiver->hopper_state = SubGhzHopperStateOFF;
                    } else {
                        osTimerStart(subghz_receiver->timer, 1024 / 10);
                        subghz_receiver->hopper_state = SubGhzHopperStateRunnig;
                    }
                    if(subghz_history_get_item(model->history) == 0) {
                        model->scene = ReceiverSceneStart;
                    } else {
                        model->scene = ReceiverSceneMain;
                    }
                    return true;
                });
            subghz_receiver->callback(SubghzReceverEventMain, subghz_receiver->context);
        } else {
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    bool model_updated = false;

                    if(event->key == InputKeyLeft) {
                        if(model->frequency > 0) model->frequency--;
                        model_updated = true;
                    } else if(event->key == InputKeyRight) {
                        if(model->frequency < subghz_frequencies_count) model->frequency++;
                        model_updated = true;
                    }
                    if(model_updated) {
                        model->real_frequency = subghz_frequencies[model->frequency];
                    }
                    return model_updated;
                });
        }
        break;

    case ReceiverSceneStart:
        if(event->type != InputTypeShort) return false;
        if(event->key == InputKeyBack) {
            return false;
        } else if(event->key == InputKeyLeft) {
            subghz_receiver->hopper_state = SubGhzHopperStatePause;
            with_view_model(
                subghz_receiver->view, (SubghzReceiverModel * model) {
                    model->temp_frequency = model->frequency;
                    model->scene = ReceiverSceneConfig;
                    return true;
                });
            subghz_receiver->callback(SubghzReceverEventConfig, subghz_receiver->context);
        }
        break;

    default:
        break;
    }

    subghz_receiver_update_offset(subghz_receiver);
    if(scene != ReceiverSceneInfo) {
        with_view_model(
            subghz_receiver->view, (SubghzReceiverModel * model) {
                if(subghz_history_get_text_space_left(model->history, NULL)) {
                    subghz_receiver_history_full(subghz_receiver);
                }
                return false;
            });
    }

    return true;
}

void subghz_receiver_text_callback(string_t text, void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_set(model->text, text);
            model->scene = ReceiverSceneMain;
            return true;
        });
}

void subghz_receiver_protocol_callback(SubGhzProtocolCommon* parser, void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            model->protocol_result = parser;
            subghz_history_set_frequency_preset(
                model->history,
                model->history_item,
                model->real_frequency,
                FuriHalSubGhzPresetOok650Async);
            subghz_history_add_to_history(model->history, parser);

            model->history_item = subghz_history_get_item(model->history);
            model->scene = ReceiverSceneMain;
            if(subghz_history_get_text_space_left(model->history, NULL)) {
                subghz_receiver_history_full(subghz_receiver);
            }
            return true;
        });
    subghz_protocol_reset(subghz_receiver->protocol);
    subghz_receiver_update_offset(subghz_receiver);
}

static void subghz_receiver_timer_callback(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;

    switch(subghz_receiver->hopper_state) {
    case SubGhzHopperStatePause:
        return;
        break;
    case SubGhzHopperStateOFF:
        osTimerStop(subghz_receiver->timer);
        return;
        break;
    case SubGhzHopperStateRSSITimeOut:
        if(subghz_receiver->hopper_timeout != 0) {
            subghz_receiver->hopper_timeout--;
            return;
        }
        break;
    default:
        break;
    }
    float rssi = -127.0f;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            if(subghz_receiver->hopper_state != SubGhzHopperStateRSSITimeOut) {
                // See RSSI Calculation timings in CC1101 17.3 RSSI
                rssi = furi_hal_subghz_get_rssi();

                // Stay if RSSI is high enough
                if(rssi > -90.0f) {
                    subghz_receiver->hopper_timeout = 10;
                    subghz_receiver->hopper_state = SubGhzHopperStateRSSITimeOut;
                    return false;
                }
            } else {
                subghz_receiver->hopper_state = SubGhzHopperStateRunnig;
            }

            // Select next frequency
            if(model->frequency < COUNT_FREQUNCY_HOPPER - 1) {
                model->frequency++;
            } else {
                model->frequency = 0;
            }

            // Restart radio
            furi_hal_subghz_idle();
            subghz_protocol_reset(subghz_receiver->protocol);
            model->real_frequency = furi_hal_subghz_set_frequency_and_path(
                subghz_frequencies_hopper[model->frequency]);
            furi_hal_subghz_rx();

            return true;
        });
}

void subghz_receiver_enter(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;
    //Start CC1101 Rx
    subghz_begin(FuriHalSubGhzPresetOok650Async);
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            subghz_rx_end(subghz_receiver->worker);
            model->frequency = subghz_frequencies_433_92;
            model->real_frequency =
                subghz_rx(subghz_receiver->worker, subghz_frequencies[model->frequency]);
            if(subghz_history_get_item(model->history) == 0) {
                model->scene = ReceiverSceneStart;
            } else {
                model->scene = ReceiverSceneMain;
            }
            return true;
        });
    subghz_protocol_enable_dump(
        subghz_receiver->protocol, subghz_receiver_protocol_callback, subghz_receiver);
}

void subghz_receiver_exit(void* context) {
    furi_assert(context);
    SubghzReceiver* subghz_receiver = context;
    osTimerStop(subghz_receiver->timer);
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_clean(model->text);
            return true;
        });
    // Stop CC1101 Rx
    subghz_rx_end(subghz_receiver->worker);
    subghz_sleep();
}

SubghzReceiver* subghz_receiver_alloc() {
    SubghzReceiver* subghz_receiver = furi_alloc(sizeof(SubghzReceiver));

    // View allocation and configuration
    subghz_receiver->view = view_alloc();
    view_allocate_model(subghz_receiver->view, ViewModelTypeLocking, sizeof(SubghzReceiverModel));
    view_set_context(subghz_receiver->view, subghz_receiver);
    view_set_draw_callback(subghz_receiver->view, (ViewDrawCallback)subghz_receiver_draw);
    view_set_input_callback(subghz_receiver->view, subghz_receiver_input);
    view_set_enter_callback(subghz_receiver->view, subghz_receiver_enter);
    view_set_exit_callback(subghz_receiver->view, subghz_receiver_exit);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_init(model->text);
            model->history = subghz_history_alloc();
            return true;
        });

    subghz_receiver->timer =
        osTimerNew(subghz_receiver_timer_callback, osTimerPeriodic, subghz_receiver, NULL);
    subghz_receiver->hopper_state = SubGhzHopperStateOFF;
    return subghz_receiver;
}

void subghz_receiver_free(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);

    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            string_clear(model->text);
            subghz_history_free(model->history);
            return false;
        });
    osTimerDelete(subghz_receiver->timer);
    view_free(subghz_receiver->view);
    free(subghz_receiver);
}

View* subghz_receiver_get_view(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    return subghz_receiver->view;
}

uint32_t subghz_receiver_get_frequency(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    uint32_t frequency;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            frequency = subghz_history_get_frequency(model->history, model->idx);
            return false;
        });
    return frequency;
}

FuriHalSubGhzPreset subghz_receiver_get_preset(SubghzReceiver* subghz_receiver) {
    furi_assert(subghz_receiver);
    FuriHalSubGhzPreset preset;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            preset = subghz_history_get_preset(model->history, model->idx);
            return false;
        });
    return preset;
}

void subghz_receiver_frequency_preset_to_str(SubghzReceiver* subghz_receiver, string_t output) {
    furi_assert(subghz_receiver);
    uint32_t frequency;
    uint32_t preset;
    with_view_model(
        subghz_receiver->view, (SubghzReceiverModel * model) {
            frequency = subghz_history_get_frequency(model->history, model->idx);
            preset = (uint32_t)subghz_history_get_preset(model->history, model->idx);
            return false;
        });

    string_cat_printf(
        output,
        "Frequency: %d\n"
        "Preset: %d\n",
        (int)frequency,
        (int)preset);
}
