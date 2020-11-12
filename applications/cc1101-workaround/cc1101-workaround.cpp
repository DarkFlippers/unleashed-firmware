#include "flipper.h"

#include "cc1101-workaround/cc1101.h"

#define RSSI_DELAY 5000 //rssi delay in micro second
#define CHAN_SPA 0.05 // channel spacing
#define F_OSC 26e6

int16_t rssi_to_dbm(uint8_t rssi_dec, uint8_t rssiOffset) {
    int16_t rssi;

    if(rssi_dec >= 128) {
        rssi = (int16_t)((int16_t)(rssi_dec - 256) / 2) - rssiOffset;
    } else {
        rssi = (rssi_dec / 2) - rssiOffset;
    }

    return rssi;
}

typedef struct {
    float base_freq;
    uint8_t reg[3]; // FREQ2, FREQ1, FREQ0
    uint8_t first_channel;
    uint8_t last_channel;
    uint8_t rssi_offset;
} Band;

typedef struct {
    const Band* band;
    uint16_t channel;
} FreqConfig;

void setup_freq(CC1101* cc1101, const FreqConfig* config) {
    // cc1101->SpiWriteReg(CC1101_MCSM0, 0x08); // disalbe FS_AUTOCAL
    cc1101->SpiWriteReg(CC1101_AGCCTRL2, 0x43 | 0x0C); // MAX_DVGA_GAIN to 11 for fast rssi
    cc1101->SpiWriteReg(CC1101_AGCCTRL0, 0xB0); // max AGC WAIT_TIME; 0 filter_length
    cc1101->SetMod(GFSK); // set to GFSK for fast rssi measurement | +8 is dcfilter off

    uint32_t freq_reg = config->band->base_freq * 1e6 / (F_OSC / 65536);
    cc1101->SetFreq((freq_reg >> 16) & 0xFF, (freq_reg >> 8) & 0xFF, (freq_reg)&0xFF);
    cc1101->SetChannel(config->channel);

    /*
    //set test0 to 0x09
    cc1101->SpiWriteReg(CC1101_TEST0, 0x09);
    //set FSCAL2 to 0x2A to force VCO HIGH
    cc1101->SpiWriteReg(CC1101_FSCAL2, 0x2A);

    // perform a manual calibration by issuing SCAL command
    cc1101->SpiStrobe(CC1101_SCAL);
    */
}

int16_t rx_rssi(CC1101* cc1101, const FreqConfig* config) {
    cc1101->SetReceive();

    delay_us(RSSI_DELAY);

    // 1.4.8) read PKTSTATUS register while the radio is in RX state
    /*uint8_t _pkt_status = */ cc1101->SpiReadStatus(CC1101_PKTSTATUS);

    // 1.4.9) enter IDLE state by issuing a SIDLE command
    cc1101->SpiStrobe(CC1101_SIDLE);

    // //read rssi value and converto to dBm form
    uint8_t rssi_dec = (uint8_t)cc1101->SpiReadStatus(CC1101_RSSI);
    int16_t rssi_dBm = rssi_to_dbm(rssi_dec, config->band->rssi_offset);

    return rssi_dBm;
}

void tx(CC1101* cc1101, const FreqConfig* config) {
    /*
    cc1101->SpiWriteReg(CC1101_MCSM0, 0x18); //enable FS_AUTOCAL
    cc1101->SpiWriteReg(CC1101_AGCCTRL2, 0x43); //back to recommended config
    cc1101->SpiWriteReg(CC1101_AGCCTRL0, 0x91); //back to recommended config
    */

    uint32_t freq_reg = config->band->base_freq * 1e6 / (F_OSC / 65536);
    cc1101->SetFreq((freq_reg >> 16) & 0xFF, (freq_reg >> 8) & 0xFF, (freq_reg)&0xFF);
    cc1101->SetChannel(config->channel);

    cc1101->SetTransmit();
}

void idle(CC1101* cc1101) {
    cc1101->SpiStrobe(CC1101_SIDLE);
}

// f = (f_osc/65536) * (FREQ + CHAN * (256 + CH_SP_M) * 2^(CH_SP_E - 2))
// FREQ = f / (f_osc/65536)
// CHAN = 0
// TODO: CHAN number not implemented!
// TODO: reg values not affetcts

const Band bands[] = {
    {300., {0x00, 0x00, 0x00}, 0, 255, 74},
    {315., {0x00, 0x00, 0x00}, 0, 255, 74},
    {348., {0x00, 0x00, 0x00}, 0, 255, 74},
    {387., {0x00, 0x00, 0x00}, 0, 255, 74},
    {433., {0x00, 0x00, 0x00}, 0, 255, 74},
    {464., {0x00, 0x00, 0x00}, 0, 255, 74},
    {779., {0x00, 0x00, 0x00}, 0, 255, 74},
    {868., {0x00, 0x00, 0x00}, 0, 255, 74},
    {915., {0x00, 0x00, 0x00}, 0, 255, 74},
    {928., {0x00, 0x00, 0x00}, 0, 255, 74},
};

const FreqConfig FREQ_LIST[] = {
    {&bands[0], 0},
    {&bands[1], 0},
    {&bands[2], 0},
    {&bands[3], 0},
    {&bands[4], 0},
    {&bands[5], 0},
    {&bands[6], 0},
    {&bands[7], 0},
    {&bands[8], 0},
    {&bands[9], 0},
};

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

typedef enum { ModeRx, ModeTx } Mode;

typedef struct {
    int16_t dbm;
    uint8_t reg;
} TxLevel;

const TxLevel TX_LEVELS[] = {
    {-10, 0},
    {-5, 0},
    {0, 0},
    {5, 0},
};

typedef struct {
    Mode mode;
    size_t active_freq;
    int16_t last_rssi;
    size_t tx_level;
    bool need_cc1101_conf;
} State;

static void render_callback(CanvasApi* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas->clear(canvas);
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);
    canvas->draw_str(canvas, 2, 12, "cc1101 workaround");

    {
        char buf[24];
        FreqConfig conf = FREQ_LIST[state->active_freq];
        float freq = conf.band->base_freq + CHAN_SPA * conf.channel;
        sprintf(buf, "freq: %ld.%02ld MHz", (uint32_t)freq, (uint32_t)(freq * 100.) % 100);

        canvas->set_font(canvas, FontSecondary);
        canvas->draw_str(canvas, 2, 25, buf);
    }

    {
        canvas->set_font(canvas, FontSecondary);

        if(state->need_cc1101_conf) {
            canvas->draw_str(canvas, 2, 36, "mode: configuring...");
        } else if(state->mode == ModeRx) {
            canvas->draw_str(canvas, 2, 36, "mode: RX");
        } else if(state->mode == ModeTx) {
            canvas->draw_str(canvas, 2, 36, "mode: TX");
        } else {
            canvas->draw_str(canvas, 2, 36, "mode: unknown");
        }
    }

    {
        if(!state->need_cc1101_conf && state->mode == ModeRx) {
            char buf[24];
            sprintf(buf, "RSSI: %d dBm", state->last_rssi);

            canvas->set_font(canvas, FontSecondary);
            canvas->draw_str(canvas, 2, 48, buf);
        }
    }

    {
        char buf[24];
        sprintf(buf, "tx level: %d dBm", TX_LEVELS[state->tx_level].dbm);

        canvas->set_font(canvas, FontSecondary);
        canvas->draw_str(canvas, 2, 63, buf);
    }

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = (QueueHandle_t)ctx;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

extern "C" void cc1101_workaround(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(1, sizeof(AppEvent), NULL);
    furi_check(event_queue);

    State _state;
    _state.mode = ModeRx;
    _state.active_freq = 0;
    _state.need_cc1101_conf = true;
    _state.last_rssi = 0;
    _state.tx_level = 0;

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("[cc1101] cannot create mutex\n");
        furiac_exit(NULL);
    }

    Widget* widget = widget_alloc();

    widget_draw_callback_set(widget, render_callback, &state_mutex);
    widget_input_callback_set(widget, input_callback, event_queue);

    // Open GUI and register widget
    GuiApi* gui = (GuiApi*)furi_open("gui");
    if(gui == NULL) {
        printf("[cc1101] gui is not available\n");
        furiac_exit(NULL);
    }
    gui->add_widget(gui, widget, GuiLayerFullscreen);

    printf("[cc1101] creating device\n");
    GpioPin cs_pin = {CC1101_CS_GPIO_Port, CC1101_CS_Pin};

    // TODO open record
    GpioPin* cs_pin_record = &cs_pin;
    CC1101 cc1101(cs_pin_record);
    printf("[cc1101] init device\n");

    uint8_t address = cc1101.Init();
    if(address > 0) {
        printf("[cc1101] init done: %d\n", address);
    } else {
        printf("[cc1101] init fail\n");
        furiac_exit(NULL);
    }

    // RX filter bandwidth 58.035714(0xFD) 100k(0xCD) 200k(0x8D)
    cc1101.SpiWriteReg(CC1101_MDMCFG4, 0xCD);
    // datarate config 250kBaud  for the purpose of fast rssi measurement
    cc1101.SpiWriteReg(CC1101_MDMCFG3, 0x3B);
    // FEC preamble etc. last 2 bits for channel spacing
    cc1101.SpiWriteReg(CC1101_MDMCFG1, 0x20);
    // 50khz channel spacing
    cc1101.SpiWriteReg(CC1101_MDMCFG0, 0xF8);

    // TODO open record
    GpioPin* led_record = (GpioPin*)&led_gpio[1];

    // configure pin
    gpio_init(led_record, GpioModeOutputOpenDrain);

    const int16_t RSSI_THRESHOLD = -89;

    AppEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 150);
        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                if(event.value.input.state && event.value.input.input == InputBack) {
                    printf("[cc1101] bye!\n");
                    // TODO remove all widgets create by app
                    widget_enabled_set(widget, false);
                    furiac_exit(NULL);
                }

                if(event.value.input.state && event.value.input.input == InputUp) {
                    if(state->active_freq > 0) {
                        state->active_freq--;
                        state->need_cc1101_conf = true;
                    }
                }

                if(event.value.input.state && event.value.input.input == InputDown) {
                    if(state->active_freq < (sizeof(FREQ_LIST) / sizeof(FREQ_LIST[0]) - 1)) {
                        state->active_freq++;
                        state->need_cc1101_conf = true;
                    }
                }

                if(event.value.input.state && event.value.input.input == InputLeft) {
                    if(state->tx_level < (sizeof(TX_LEVELS) / sizeof(TX_LEVELS[0]) - 1)) {
                        state->tx_level++;
                    } else {
                        state->tx_level = 0;
                    }

                    state->need_cc1101_conf = true;
                }

                if(event.value.input.input == InputOk) {
                    state->mode = event.value.input.state ? ModeTx : ModeRx;
                    state->need_cc1101_conf = true;
                }
            }
        } else {
            if(!state->need_cc1101_conf && state->mode == ModeRx) {
                state->last_rssi = rx_rssi(&cc1101, &FREQ_LIST[state->active_freq]);
            }
        }

        if(state->need_cc1101_conf) {
            if(state->mode == ModeRx) {
                setup_freq(&cc1101, &FREQ_LIST[state->active_freq]);
                state->last_rssi = rx_rssi(&cc1101, &FREQ_LIST[state->active_freq]);
                // idle(&cc1101);
            } else if(state->mode == ModeTx) {
                tx(&cc1101, &FREQ_LIST[state->active_freq]);
            }

            state->need_cc1101_conf = false;
        }

        gpio_write(
            led_record,
            (state->last_rssi > RSSI_THRESHOLD && !state->need_cc1101_conf) ? false : true);

        release_mutex(&state_mutex, state);
        widget_update(widget);
    }
}