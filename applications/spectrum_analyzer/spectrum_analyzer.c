#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include "spectrum_analyzer.h"

#include <lib/drivers/cc1101_regs.h>
#include "spectrum_analyzer_worker.h"

typedef struct {
    uint16_t center_freq;
    uint8_t width;
    uint8_t band;
    uint8_t vscroll;

    uint32_t channel0_frequency;
    uint32_t spacing;

    bool mode_change;

    float max_rssi;
    uint8_t max_rssi_dec;
    uint8_t max_rssi_channel;
    uint8_t channel_ss[NUM_CHANNELS];
} SpectrumAnalyzerModel;

typedef struct {
    SpectrumAnalyzerModel* model;
    osMutexId_t* model_mutex;

    osMessageQueueId_t event_queue;

    ViewPort* view_port;
    Gui* gui;

    SpectrumAnalyzerWorker* worker;
} SpectrumAnalyzer;

void spectrum_analyzer_draw_scale(Canvas* canvas, const SpectrumAnalyzerModel* model) {
    // Draw line
    canvas_draw_line(
        canvas, FREQ_START_X, FREQ_BOTTOM_Y, FREQ_START_X + FREQ_LENGTH_X, FREQ_BOTTOM_Y);
    // Draw minor scale
    for(int i = FREQ_START_X; i < FREQ_START_X + FREQ_LENGTH_X; i += 5) {
        canvas_draw_line(canvas, i, FREQ_BOTTOM_Y, i, FREQ_BOTTOM_Y + 2);
    }
    // Draw major scale
    for(int i = FREQ_START_X; i < FREQ_START_X + FREQ_LENGTH_X; i += 25) {
        canvas_draw_line(canvas, i, FREQ_BOTTOM_Y, i, FREQ_BOTTOM_Y + 4);
    }

    // Draw scale tags
    uint16_t tag_left;
    uint16_t tag_center;
    uint16_t tag_right;
    char temp_str[18];

    tag_center = model->center_freq;

    switch(model->width) {
    case NARROW:
        tag_left = model->center_freq - 2;
        tag_right = model->center_freq + 2;
        break;
    case ULTRANARROW:
        tag_left = model->center_freq - 1;
        tag_right = model->center_freq + 1;
        break;
    case ULTRAWIDE:
        tag_left = model->center_freq - 40;
        tag_right = model->center_freq + 40;
        break;
    default:
        tag_left = model->center_freq - 10;
        tag_right = model->center_freq + 10;
    }

    canvas_set_font(canvas, FontSecondary);
    snprintf(temp_str, 18, "%u", tag_left);
    canvas_draw_str_aligned(canvas, FREQ_START_X, 63, AlignCenter, AlignBottom, temp_str);
    snprintf(temp_str, 18, "%u", tag_center);
    canvas_draw_str_aligned(canvas, 128 / 2, 63, AlignCenter, AlignBottom, temp_str);
    snprintf(temp_str, 18, "%u", tag_right);
    canvas_draw_str_aligned(
        canvas, FREQ_START_X + FREQ_LENGTH_X - 1, 63, AlignCenter, AlignBottom, temp_str);
}

static void spectrum_analyzer_render_callback(Canvas* const canvas, void* ctx) {
    SpectrumAnalyzer* spectrum_analyzer = ctx;
    //furi_check(osMutexAcquire(spectrum_analyzer->model_mutex, osWaitForever) == osOK);

    SpectrumAnalyzerModel* model = spectrum_analyzer->model;

    spectrum_analyzer_draw_scale(canvas, model);

    for(uint8_t column = 0; column < 128; column++) {
        uint8_t ss = model->channel_ss[column + 2];
        // Compress height to max of 64 values (255>>2)
        uint8_t s = MAX((ss - model->vscroll) >> 2, 0);
        uint8_t y = FREQ_BOTTOM_Y - s; // bar height

        // Draw each bar
        canvas_draw_line(canvas, column, FREQ_BOTTOM_Y, column, y);
    }

    if(model->mode_change) {
        char temp_mode_str[12];
        switch(model->width) {
        case NARROW:
            strncpy(temp_mode_str, "NARROW", 12);
            break;
        case ULTRANARROW:
            strncpy(temp_mode_str, "ULTRANARROW", 12);
            break;
        case ULTRAWIDE:
            strncpy(temp_mode_str, "ULTRAWIDE", 12);
            break;
        default:
            strncpy(temp_mode_str, "WIDE", 12);
            break;
        }

        // Current mode label
        char tmp_str[21];
        snprintf(tmp_str, 21, "Mode: %s", temp_mode_str);
        canvas_draw_str_aligned(canvas, 127, 4, AlignRight, AlignTop, tmp_str);
    }
    // Draw cross and label
    if(model->max_rssi > PEAK_THRESHOLD) {
        // Compress height to max of 64 values (255>>2)
        uint8_t max_y = MAX((model->max_rssi_dec - model->vscroll) >> 2, 0);
        max_y = (FREQ_BOTTOM_Y - max_y);

        // Cross
        int16_t x1, x2, y1, y2;
        x1 = model->max_rssi_channel - 2 - 2;
        if(x1 < 0) x1 = 0;
        y1 = max_y - 2;
        if(y1 < 0) y1 = 0;
        x2 = model->max_rssi_channel - 2 + 2;
        if(x2 > 127) x2 = 127;
        y2 = max_y + 2;
        if(y2 > 63) y2 = 63; // SHOULD NOT HAPPEN CHECK!
        canvas_draw_line(canvas, x1, y1, x2, y2);

        x1 = model->max_rssi_channel - 2 + 2;
        if(x1 > 127) x1 = 127;
        y1 = max_y - 2;
        if(y1 < 0) y1 = 0;
        x2 = model->max_rssi_channel - 2 - 2;
        if(x2 < 0) x2 = 0;
        y2 = max_y + 2;
        if(y2 > 63) y2 = 63; // SHOULD NOT HAPPEN CHECK!
        canvas_draw_line(canvas, (uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2);

        // Label
        char temp_str[36];
        snprintf(
            temp_str,
            36,
            "Peak: %3.2f Mhz %3.1f dbm",
            ((double)(model->channel0_frequency + (model->max_rssi_channel * model->spacing)) /
             1000000),
            (double)model->max_rssi);
        canvas_draw_str_aligned(canvas, 127, 0, AlignRight, AlignTop, temp_str);
    }

    //osMutexRelease(spectrum_analyzer->model_mutex);

    // FURI_LOG_D("Spectrum", "model->vscroll %u", model->vscroll);
}

static void spectrum_analyzer_input_callback(InputEvent* input_event, void* ctx) {
    SpectrumAnalyzer* spectrum_analyzer = ctx;
    // Only handle short presses
    if(input_event->type == InputTypeShort) {
        osMessageQueuePut(spectrum_analyzer->event_queue, input_event, 0, osWaitForever);
    }
}

static void spectrum_analyzer_worker_callback(
    void* channel_ss,
    float max_rssi,
    uint8_t max_rssi_dec,
    uint8_t max_rssi_channel,
    void* context) {
    SpectrumAnalyzer* spectrum_analyzer = context;
    furi_check(osMutexAcquire(spectrum_analyzer->model_mutex, osWaitForever) == osOK);

    SpectrumAnalyzerModel* model = (SpectrumAnalyzerModel*)spectrum_analyzer->model;
    memcpy(model->channel_ss, (uint8_t*)channel_ss, sizeof(uint8_t) * NUM_CHANNELS);
    model->max_rssi = max_rssi;
    model->max_rssi_dec = max_rssi_dec;
    model->max_rssi_channel = max_rssi_channel;

    osMutexRelease(spectrum_analyzer->model_mutex);
    view_port_update(spectrum_analyzer->view_port);
}

void spectrum_analyzer_calculate_frequencies(SpectrumAnalyzerModel* model) {
    // REDO ALL THIS. CALCULATE ONLY WITH SPACING!

    uint8_t new_band;
    uint32_t min_hz;
    uint32_t max_hz;
    uint8_t margin;
    uint8_t step;
    uint16_t upper_limit;
    uint16_t lower_limit;
    uint16_t next_up;
    uint16_t next_down;
    uint8_t next_band_up;
    uint8_t next_band_down;

    switch(model->width) {
    case NARROW:
        margin = NARROW_MARGIN;
        step = NARROW_STEP;
        model->spacing = NARROW_SPACING;
        break;
    case ULTRANARROW:
        margin = ULTRANARROW_MARGIN;
        step = ULTRANARROW_STEP;
        model->spacing = ULTRANARROW_SPACING;
        break;
    case ULTRAWIDE:
        margin = ULTRAWIDE_MARGIN;
        step = ULTRAWIDE_STEP;
        model->spacing = ULTRAWIDE_SPACING;
        /* nearest 20 MHz step */
        model->center_freq = ((model->center_freq + 10) / 20) * 20;
        break;
    default:
        margin = WIDE_MARGIN;
        step = WIDE_STEP;
        model->spacing = WIDE_SPACING;
        /* nearest 5 MHz step */
        model->center_freq = ((model->center_freq + 2) / 5) * 5;
        break;
    }

    /* handle cases near edges of bands */
    if(model->center_freq > EDGE_900) {
        new_band = BAND_900;
        upper_limit = UPPER(MAX_900, margin, step);
        lower_limit = LOWER(MIN_900, margin, step);
        next_up = LOWER(MIN_300, margin, step);
        next_down = UPPER(MAX_400, margin, step);
        next_band_up = BAND_300;
        next_band_down = BAND_400;
    } else if(model->center_freq > EDGE_400) {
        new_band = BAND_400;
        upper_limit = UPPER(MAX_400, margin, step);
        lower_limit = LOWER(MIN_400, margin, step);
        next_up = LOWER(MIN_900, margin, step);
        next_down = UPPER(MAX_300, margin, step);
        next_band_up = BAND_900;
        next_band_down = BAND_300;
    } else {
        new_band = BAND_300;
        upper_limit = UPPER(MAX_300, margin, step);
        lower_limit = LOWER(MIN_300, margin, step);
        next_up = LOWER(MIN_400, margin, step);
        next_down = UPPER(MAX_900, margin, step);
        next_band_up = BAND_400;
        next_band_down = BAND_900;
    }

    if(model->center_freq > upper_limit) {
        model->center_freq = upper_limit;
        if(new_band == model->band) {
            new_band = next_band_up;
            model->center_freq = next_up;
        }
    } else if(model->center_freq < lower_limit) {
        model->center_freq = lower_limit;
        if(new_band == model->band) {
            new_band = next_band_down;
            model->center_freq = next_down;
        }
    }

    model->band = new_band;
    /* doing everything in Hz from here on */
    switch(model->band) {
    case BAND_400:
        min_hz = MIN_400 * 1000000;
        max_hz = MAX_400 * 1000000;
        break;
    case BAND_300:
        min_hz = MIN_300 * 1000000;
        max_hz = MAX_300 * 1000000;
        break;
    default:
        min_hz = MIN_900 * 1000000;
        max_hz = MAX_900 * 1000000;
        break;
    }

    model->channel0_frequency =
        model->center_freq * 1000000 - (model->spacing * ((NUM_CHANNELS / 2) + 1));

    // /* calibrate upper channels */
    // hz = model->center_freq * 1000000;
    // max_chan = NUM_CHANNELS / 2;
    // while (hz <= max_hz && max_chan < NUM_CHANNELS) {
    //     instance->chan_table[max_chan].frequency = hz;
    //     FURI_LOG_T("Spectrum", "calibrate_freq ch[%u]: %lu", max_chan, hz);
    //     hz += model->spacing;
    //     max_chan++;
    // }

    // /* calibrate lower channels */
    // hz = instance->freq * 1000000 - model->spacing;
    // min_chan = NUM_CHANNELS / 2;
    // while (hz >= min_hz && min_chan > 0) {
    //     min_chan--;
    //     instance->chan_table[min_chan].frequency = hz;
    //     FURI_LOG_T("Spectrum", "calibrate_freq ch[%u]: %lu", min_chan, hz);
    //     hz -= model->spacing;
    // }

    model->max_rssi = -200.0;
    model->max_rssi_dec = 0;

    FURI_LOG_D("Spectrum", "setup_frequencies - max_hz: %u - min_hz: %u", max_hz, min_hz);
    FURI_LOG_D("Spectrum", "center_freq: %u", model->center_freq);
    FURI_LOG_D(
        "Spectrum",
        "ch[0]: %lu - ch[%u]: %lu",
        model->channel0_frequency,
        NUM_CHANNELS - 1,
        model->channel0_frequency + ((NUM_CHANNELS - 1) * model->spacing));
}

SpectrumAnalyzer* spectrum_analyzer_alloc() {
    SpectrumAnalyzer* instance = malloc(sizeof(SpectrumAnalyzer));
    instance->model = malloc(sizeof(SpectrumAnalyzerModel));

    SpectrumAnalyzerModel* model = instance->model;

    for(uint8_t ch = 0; ch < NUM_CHANNELS - 1; ch++) {
        model->channel_ss[ch] = 0;
    }
    model->max_rssi_dec = 0;
    model->max_rssi_channel = 0;
    model->max_rssi = PEAK_THRESHOLD - 1; // Should initializar to < PEAK_THRESHOLD

    model->center_freq = DEFAULT_FREQ;
    model->width = WIDE;
    model->band = BAND_400;

    model->vscroll = DEFAULT_VSCROLL;

    instance->model_mutex = osMutexNew(NULL);
    instance->event_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);

    instance->worker = spectrum_analyzer_worker_alloc();

    spectrum_analyzer_worker_set_callback(
        instance->worker, spectrum_analyzer_worker_callback, instance);

    // Set system callbacks
    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, spectrum_analyzer_render_callback, instance);
    view_port_input_callback_set(instance->view_port, spectrum_analyzer_input_callback, instance);

    // Open GUI and register view_port
    instance->gui = furi_record_open("gui");
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    return instance;
}

void spectrum_analyzer_free(SpectrumAnalyzer* instance) {
    // view_port_enabled_set(view_port, false);
    gui_remove_view_port(instance->gui, instance->view_port);
    furi_record_close("gui");
    view_port_free(instance->view_port);

    spectrum_analyzer_worker_free(instance->worker);

    osMessageQueueDelete(instance->event_queue);

    osMutexDelete(instance->model_mutex);

    free(instance->model);
    free(instance);

    furi_hal_subghz_idle();
    furi_hal_subghz_sleep();
}

int32_t spectrum_analyzer_app(void* p) {
    UNUSED(p);

    SpectrumAnalyzer* spectrum_analyzer = spectrum_analyzer_alloc();
    InputEvent input;

    FURI_LOG_D("Spectrum", "Main Loop - Starting worker");
    furi_hal_delay_ms(50);

    spectrum_analyzer_worker_start(spectrum_analyzer->worker);

    FURI_LOG_D("Spectrum", "Main Loop - Wait on queue");
    furi_hal_delay_ms(50);

    while(osMessageQueueGet(spectrum_analyzer->event_queue, &input, NULL, osWaitForever) == osOK) {
        furi_check(osMutexAcquire(spectrum_analyzer->model_mutex, osWaitForever) == osOK);

        FURI_LOG_D("Spectrum", "Main Loop - Input: %u", input.key);

        SpectrumAnalyzerModel* model = spectrum_analyzer->model;

        uint8_t vstep = VERTICAL_SHORT_STEP;
        uint8_t hstep;

        bool exit_loop = false;

        switch(model->width) {
        case NARROW:
            hstep = NARROW_STEP;
            break;
        case ULTRANARROW:
            hstep = ULTRANARROW_STEP;
            break;
        case ULTRAWIDE:
            hstep = ULTRAWIDE_STEP;
            break;
        default:
            hstep = WIDE_STEP;
            break;
        }

        switch(input.key) {
        case InputKeyUp:
            model->vscroll = MAX(model->vscroll - vstep, MIN_VSCROLL);
            FURI_LOG_D("Spectrum", "Vscroll: %u", model->vscroll);
            break;
        case InputKeyDown:
            model->vscroll = MIN(model->vscroll + vstep, MAX_VSCROLL);
            FURI_LOG_D("Spectrum", "Vscroll: %u", model->vscroll);
            break;
        case InputKeyRight:
            model->center_freq += hstep;
            FURI_LOG_D("Spectrum", "center_freq: %lu", model->center_freq);
            spectrum_analyzer_calculate_frequencies(model);
            spectrum_analyzer_worker_set_frequencies(
                spectrum_analyzer->worker, model->channel0_frequency, model->spacing, model->width);
            break;
        case InputKeyLeft:
            model->center_freq -= hstep;
            spectrum_analyzer_calculate_frequencies(model);
            spectrum_analyzer_worker_set_frequencies(
                spectrum_analyzer->worker, model->channel0_frequency, model->spacing, model->width);
            FURI_LOG_D("Spectrum", "center_freq: %lu", model->center_freq);
            break;
        case InputKeyOk: {
            switch(model->width) {
            case WIDE:
                model->width = NARROW;
                break;
            case NARROW:
                model->width = ULTRANARROW;
                break;
            case ULTRANARROW:
                model->width = ULTRAWIDE;
                break;
            case ULTRAWIDE:
                model->width = WIDE;
                break;
            default:
                model->width = WIDE;
                break;
            }
        }

            model->mode_change = true;
            view_port_update(spectrum_analyzer->view_port);

            furi_hal_delay_ms(1000);

            model->mode_change = false;
            spectrum_analyzer_calculate_frequencies(model);
            spectrum_analyzer_worker_set_frequencies(
                spectrum_analyzer->worker, model->channel0_frequency, model->spacing, model->width);
            FURI_LOG_D("Spectrum", "Width: %u", model->width);
            break;
        case InputKeyBack:
            exit_loop = true;
            break;
        }

        osMutexRelease(spectrum_analyzer->model_mutex);
        view_port_update(spectrum_analyzer->view_port);
        if(exit_loop == true) break;
    }

    spectrum_analyzer_worker_stop(spectrum_analyzer->worker);

    spectrum_analyzer_free(spectrum_analyzer);

    return 0;
}