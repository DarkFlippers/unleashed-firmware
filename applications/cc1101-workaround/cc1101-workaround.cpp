#include "flipper.h"

#include "cc1101-workaround/cc1101.h"

extern "C" void cli_print(const char* str);

#define RSSI_DELAY 5000 //rssi delay in micro second
#define CHAN_SPA 0.05 // channel spacing

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

void setup_freq(CC1101* cc1101, float freq) {
    // cc1101->SpiWriteReg(CC1101_MCSM0, 0x08); // disalbe FS_AUTOCAL
    // cc1101->SpiWriteReg(CC1101_AGCCTRL2, 0x43 | 0x0C); // MAX_DVGA_GAIN to 11 for fast rssi
    // cc1101->SpiWriteReg(CC1101_AGCCTRL0, 0xB0); // max AGC WAIT_TIME; 0 filter_length
    // cc1101->SetMod(GFSK); // set to GFSK for fast rssi measurement | +8 is dcfilter off

    uint32_t freq_reg = freq * 1e6 / (F_OSC / 65536);
    cc1101->SetFreq((freq_reg >> 16) & 0xFF, (freq_reg >> 8) & 0xFF, (freq_reg)&0xFF);
    cc1101->SetChannel(0);

    /*
    //set test0 to 0x09
    cc1101->SpiWriteReg(CC1101_TEST0, 0x09);
    //set FSCAL2 to 0x2A to force VCO HIGH
    cc1101->SpiWriteReg(CC1101_FSCAL2, 0x2A);

    // perform a manual calibration by issuing SCAL command
    cc1101->SpiStrobe(CC1101_SCAL);
    */
}

static GpioPin debug_0 = {GPIOB, GPIO_PIN_2};

int16_t rx_rssi(CC1101* cc1101, const FreqConfig* config) {
    // cc1101->SpiStrobe(CC1101_SFRX);
    // cc1101->SetReceive();

    // uint8_t begin_size = cc1101->SpiReadStatus(CC1101_RXBYTES);
    // uint8_t rx_status = cc1101->SpiReadStatus(CC1101_MARCSTATE);

    // delay_us(RSSI_DELAY);
    // osDelay(15);

    // uint8_t end_size = cc1101->SpiReadStatus(CC1101_RXBYTES);

    // 1.4.8) read PKTSTATUS register while the radio is in RX state
    /*uint8_t _pkt_status = */ // cc1101->SpiReadStatus(CC1101_PKTSTATUS);

    // 1.4.9) enter IDLE state by issuing a SIDLE command
    // cc1101->SpiStrobe(CC1101_SIDLE);

    // //read rssi value and converto to dBm form
    uint8_t rssi_dec = (uint8_t)cc1101->SpiReadStatus(CC1101_RSSI);
    int16_t rssi_dBm = rssi_to_dbm(rssi_dec, config->band->rssi_offset);

    /*
    char buf[256];
    sprintf(buf, "status: %d -> %d, rssi: %d\n", rx_status, cc1101->SpiReadStatus(CC1101_MARCSTATE), rssi_dBm);
    cli_print(buf);
    sprintf(buf, "begin: %d, end: %d\n", begin_size, end_size);
    cli_print(buf);
    */

    // uint8_t rx_data[64];
    // uint8_t fifo_length = end_size - begin_size;

    /*
    if(fifo_length < 64) {
        // cc1101->SpiReadBurstReg(CC1101_RXFIFO, rx_data, fifo_length);

        *
        printf("FIFO:");
        for(uint8_t i = 0; i < fifo_length; i++) {
            for(uint8_t bit = 0; bit < 8; bit++) {
                printf("%s", (rx_data[i] & (1 << bit)) > 0 ? "1" : "0");
            }
            printf(" ");
        }
        printf("\n");
        *

        for(uint8_t i = 0; i < fifo_length; i++) {
            for(uint8_t bit = 0; bit < 8; bit++) {
                gpio_write((GpioPin*)&debug_0, (rx_data[i] & (1 << bit)) > 0);
                delay_us(5);
            }
        }
    } else {
        cli_print("fifo size over\n");
    }
    */

    return rssi_dBm;
}

/*
void flp_config(CC1101* cc1101) {
    cc1101->SpiWriteReg(
        CC1101_MCSM0, 0x18); // calibrate when going from IDLE to RX or TX ; 149 - 155 μs timeout
    // MCSM0.FS_AUTOCAL[1:0] = 1

    cc1101->SpiWriteReg(CC1101_AGCCTRL2, 0x43);
    cc1101->SpiWriteReg(CC1101_AGCCTRL1, 0x49);
    cc1101->SpiWriteReg(CC1101_AGCCTRL0, 0x91);

    //freq synthesizer calibration
    cc1101->SpiWriteReg(CC1101_FSCAL3, 0xEA);
    cc1101->SpiWriteReg(CC1101_FSCAL2, 0x2A);
    cc1101->SpiWriteReg(CC1101_FSCAL1, 0x00);
    cc1101->SpiWriteReg(CC1101_FSCAL0, 0x1F);

    // async data out
    cc1101->SpiSetRegValue(CC1101_IOCFG0, 13, 5, 0); // GDO0 Output Pin Configuration
    cc1101->SpiSetRegValue(CC1101_IOCFG0, 13, 5, 0); // WAT

    // FIFOTHR.ADC_RETENTION = 1
    cc1101->SpiSetRegValue(CC1101_FIFOTHR, 1, 6, 6);

    // PKTCTRL1.APPEND_STATUS = 0
    cc1101->SpiSetRegValue(CC1101_PKTCTRL1, 0, 2, 2);

    // PKTCTRL0.WHITE_DATA = 0
    cc1101->SpiSetRegValue(CC1101_PKTCTRL0, 0, 6, 6);

    // PKTCTRL0.LENGTH_CONFIG = 2 // Infinite packet length mode
    cc1101->SpiSetRegValue(CC1101_PKTCTRL0, 2, 1, 0);

    // PKTCTRL0.CRC_EN = 0
    cc1101->SpiSetRegValue(CC1101_PKTCTRL0, 0, 2, 2);

    // PKTCTRL0.PKT_FORMAT = 3
    cc1101->SpiSetRegValue(CC1101_PKTCTRL0, 3, 5, 4);

    // bandwidth 50-100 kHz
    if(!cc1101->setRxBandwidth(75.0)) {
        printf("wrong rx bw\n");
    }

    // datarate ~30 kbps
    if(!cc1101->setBitRate(100.)) {
        printf("wrong bitrate\n");
    }

    // mod
    // MDMCFG2.MOD_FORMAT = 3 (3: OOK, 0: 2-FSK)
    cc1101->SpiSetRegValue(CC1101_MDMCFG2, 3, 6, 4);
    // MDMCFG2.SYNC_MODE = 0
    cc1101->SpiSetRegValue(CC1101_MDMCFG2, 0, 2, 0);
}
*/

void tx_config(CC1101* cc1101) {
    // cc1101->SpiWriteReg(CC1101_IOCFG2,0x0B);  //GDO2 Output Pin Configuration
    // cc1101->SpiWriteReg(CC1101_IOCFG0,0x0C);  //GDO0 Output Pin Configuration
    cc1101->SpiSetRegValue(CC1101_IOCFG0, 13, 5, 0); // GDO0 Output Pin Configuration

    cc1101->SpiWriteReg(CC1101_FIFOTHR, 0x47); //RX FIFO and TX FIFO Thresholds
    cc1101->SpiWriteReg(CC1101_PKTCTRL0, 0x32); //Packet Automation Control
    cc1101->SpiWriteReg(CC1101_FSCTRL1, 0x06); //Frequency Synthesizer Control
    cc1101->SpiWriteReg(CC1101_FREQ2, 0x10); //Frequency Control Word, High Byte
    cc1101->SpiWriteReg(CC1101_FREQ1, 0xB0); //Frequency Control Word, Middle Byte
    cc1101->SpiWriteReg(CC1101_FREQ0, 0x71); //Frequency Control Word, Low Byte
    cc1101->SpiWriteReg(CC1101_MDMCFG4, 0x6A); //Modem Configuration
    cc1101->SpiWriteReg(CC1101_MDMCFG3, 0x2E); //Modem Configuration
    cc1101->SpiWriteReg(CC1101_MDMCFG2, 0x30); //Modem Configuration
    cc1101->SpiWriteReg(CC1101_DEVIATN, 0x15); //Modem Deviation Setting
    cc1101->SpiWriteReg(CC1101_MCSM0, 0x18); //Main Radio Control State Machine Configuration
    cc1101->SpiWriteReg(CC1101_FOCCFG, 0x16); //Frequency Offset Compensation Configuration
    cc1101->SpiWriteReg(CC1101_WORCTRL, 0xFB); //Wake On Radio Control
    cc1101->SpiWriteReg(CC1101_FREND0, 0x11); //Front End TX Configuration
    cc1101->SpiWriteReg(CC1101_FSCAL3, 0xE9); //Frequency Synthesizer Calibration
    cc1101->SpiWriteReg(CC1101_FSCAL2, 0x2A); //Frequency Synthesizer Calibration
    cc1101->SpiWriteReg(CC1101_FSCAL1, 0x00); //Frequency Synthesizer Calibration
    cc1101->SpiWriteReg(CC1101_FSCAL0, 0x1F); //Frequency Synthesizer Calibration

    /*
    cc1101->SpiWriteReg(CC1101_TEST2, 0x81); //Various Test Settings
    cc1101->SpiWriteReg(CC1101_TEST1, 0x35); //Various Test Settings
    cc1101->SpiWriteReg(CC1101_TEST0, 0x09); //Various Test Settings
    */
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
    {386., {0x00, 0x00, 0x00}, 0, 255, 74},
    {433.92, {0x00, 0x00, 0x00}, 0, 255, 74},
    {438.9, {0x00, 0x00, 0x00}, 0, 255, 74},
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
    {&bands[10], 0},
};

extern "C" void cc1101_isr() {
    gpio_write((GpioPin*)&debug_0, gpio_read(&cc1101_g0_gpio));
}

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
    size_t active_freq_idx;
    float active_freq;
    int16_t last_rssi;
    size_t tx_level;
    bool need_cc1101_conf;
} State;

static void render_callback(Canvas* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    if(!state) return;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "cc1101 workaround");

    {
        char buf[24];
        sprintf(
            buf,
            "freq: %ld.%02ld MHz",
            (uint32_t)state->active_freq,
            (uint32_t)(state->active_freq * 100.) % 100);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 25, buf);
    }

    {
        canvas_set_font(canvas, FontSecondary);

        if(state->need_cc1101_conf) {
            canvas_draw_str(canvas, 2, 36, "mode: configuring...");
        } else if(state->mode == ModeRx) {
            canvas_draw_str(canvas, 2, 36, "mode: RX");
        } else if(state->mode == ModeTx) {
            canvas_draw_str(canvas, 2, 36, "mode: TX");
        } else {
            canvas_draw_str(canvas, 2, 36, "mode: unknown");
        }
    }

    {
        if(!state->need_cc1101_conf && state->mode == ModeRx) {
            char buf[24];
            sprintf(buf, "RSSI: %d dBm", state->last_rssi);

            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 2, 48, buf);
        }
    }

    {
        char buf[24];
        sprintf(buf, "tx level: %d dBm", TX_LEVELS[state->tx_level].dbm);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 63, buf);
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
    _state.active_freq_idx = 4;

    FreqConfig conf = FREQ_LIST[_state.active_freq_idx];
    _state.active_freq = conf.band->base_freq + CHAN_SPA * conf.channel;

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
    Gui* gui = (Gui*)furi_open("gui");
    if(gui == NULL) {
        printf("[cc1101] gui is not available\n");
        furiac_exit(NULL);
    }
    gui_add_widget(gui, widget, GuiLayerFullscreen);

    gpio_init(&debug_0, GpioModeOutputPushPull);
    gpio_write((GpioPin*)&debug_0, false);

    printf("[cc1101] creating device\n");
    GpioPin cs_pin = {CC1101_CS_GPIO_Port, CC1101_CS_Pin};

    gpio_init(&cc1101_g0_gpio, GpioModeInput);

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

    cc1101.SpiStrobe(CC1101_SIDLE);

    // flp_config(&cc1101);
    tx_config(&cc1101);
    // setup_freq(&cc1101, &FREQ_LIST[4]);
    // enable_cc1101_irq();

    printf("init ok\n");

    // TODO open record
    GpioPin* led_record = (GpioPin*)&led_gpio[1];

    // configure pin
    gpio_init(led_record, GpioModeOutputOpenDrain);

    const int16_t RSSI_THRESHOLD = -60;

    // setup_freq(&cc1101, &FREQ_LIST[1]);

    cc1101.SetReceive();

    AppEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                if(event.value.input.state && event.value.input.input == InputBack) {
                    printf("[cc1101] bye!\n");
                    // TODO remove all widgets create by app
                    widget_enabled_set(widget, false);
                    furiac_exit(NULL);
                }

                if(event.value.input.state && event.value.input.input == InputDown) {
                    if(state->active_freq_idx > 0) {
                        state->active_freq_idx--;
                    }

                    FreqConfig conf = FREQ_LIST[state->active_freq_idx];
                    state->active_freq = conf.band->base_freq + CHAN_SPA * conf.channel;
                    state->need_cc1101_conf = true;
                }

                if(event.value.input.state && event.value.input.input == InputUp) {
                    if(state->active_freq_idx < (sizeof(FREQ_LIST) / sizeof(FREQ_LIST[0]) - 1)) {
                        state->active_freq_idx++;
                    }

                    FreqConfig conf = FREQ_LIST[state->active_freq_idx];
                    state->active_freq = conf.band->base_freq + CHAN_SPA * conf.channel;
                    state->need_cc1101_conf = true;
                }

                if(event.value.input.state && event.value.input.input == InputRight) {
                    /*
                    if(state->tx_level < (sizeof(TX_LEVELS) / sizeof(TX_LEVELS[0]) - 1)) {
                        state->tx_level++;
                    } else {
                        state->tx_level = 0;
                    }
                    */

                    state->active_freq += 0.25;
                    state->need_cc1101_conf = true;
                }

                if(event.value.input.state && event.value.input.input == InputLeft) {
                    /*
                    if(state->tx_level < (sizeof(TX_LEVELS) / sizeof(TX_LEVELS[0]) - 1)) {
                        state->tx_level++;
                    } else {
                        state->tx_level = 0;
                    }
                    */

                    state->active_freq -= 0.25;
                    state->need_cc1101_conf = true;
                }

                if(event.value.input.input == InputOk) {
                    state->mode = event.value.input.state ? ModeTx : ModeRx;
                    state->need_cc1101_conf = true;
                }
            }
        } else {
        }

        if(state->need_cc1101_conf) {
            if(state->mode == ModeRx) {
                cc1101.SpiStrobe(CC1101_SIDLE);
                gpio_init(&cc1101_g0_gpio, GpioModeInput);

                setup_freq(&cc1101, state->active_freq);
                cc1101.SetReceive();

                state->last_rssi = rx_rssi(&cc1101, &FREQ_LIST[state->active_freq_idx]);
            } else if(state->mode == ModeTx) {
                cc1101.SpiStrobe(CC1101_SIDLE);

                setup_freq(&cc1101, state->active_freq);
                cc1101.SetTransmit();
                gpio_init(&cc1101_g0_gpio, GpioModeOutputPushPull);
                gpio_write(&cc1101_g0_gpio, false);
            }

            state->need_cc1101_conf = false;
        }

        if(!state->need_cc1101_conf && state->mode == ModeRx) {
            // TOOD what about rssi offset
            state->last_rssi = rx_rssi(&cc1101, &FREQ_LIST[state->active_freq_idx]);

            gpio_write(led_record, state->last_rssi < RSSI_THRESHOLD);
        } else if(!state->need_cc1101_conf && state->mode == ModeTx) {
            /*
            const uint8_t data = 0xA5;

            for(uint8_t i = 0; i < 8; i++) {
                gpio_write(&cc1101_g0_gpio, (data & (1 << i)) > 0);
                osDelay(1);
            }
            gpio_write(&cc1101_g0_gpio, false);
            */

            /*
            // BELL UDB-Q022-0000
            const uint16_t HALF_PERIOD = 500;

            for(uint8_t n = 0; n < 4; n++) {
                for(uint8_t i = 0; i < 4; i++) {
                    gpio_write(&cc1101_g0_gpio, true);
                    delay_us(3 * HALF_PERIOD);
                    gpio_write(&cc1101_g0_gpio, false);
                    delay_us(HALF_PERIOD);
                }

                for(uint8_t i = 0; i < 40; i++) {
                    gpio_write(&cc1101_g0_gpio, true);
                    delay_us(HALF_PERIOD);
                    gpio_write(&cc1101_g0_gpio, false);
                    delay_us(HALF_PERIOD);
                }
            }
            */

            // BELL ERA C61, static code
            const uint16_t ONE_ON = 150;
            const uint16_t ONE_OFF = 400;
            const uint16_t ZERO_ON = 420;
            const uint16_t ZERO_OFF = 130;

            const bool SEQ[] = {true,  true, false, false, true,  false, true,  false, true,
                                false, true, true,  true,  false, true,  false, true,  true,
                                true,  true, true,  false, true,  false, true};

            for(uint8_t n = 0; n < 10; n++) {
                for(uint8_t i = 0; i < sizeof(SEQ) / sizeof(SEQ[0]); i++) {
                    if(SEQ[i]) {
                        gpio_write(&cc1101_g0_gpio, false);
                        delay_us(ONE_ON);
                        gpio_write(&cc1101_g0_gpio, true);
                        delay_us(ONE_OFF);
                    } else {
                        gpio_write(&cc1101_g0_gpio, false);
                        delay_us(ZERO_ON);
                        gpio_write(&cc1101_g0_gpio, true);
                        delay_us(ZERO_OFF);
                    }
                }

                osDelay(4);
            }

            gpio_write(&cc1101_g0_gpio, false);
        }

        release_mutex(&state_mutex, state);
        widget_update(widget);
    }
}