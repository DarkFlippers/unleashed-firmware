#include "dtmf_dolphin_audio.h"

DTMFDolphinAudio* current_player;

static void dtmf_dolphin_audio_dma_isr(void* ctx) {
    FuriMessageQueue* event_queue = ctx;

    if(LL_DMA_IsActiveFlag_HT1(DMA1)) {
        LL_DMA_ClearFlag_HT1(DMA1);

        DTMFDolphinCustomEvent event = {.type = DTMFDolphinEventDMAHalfTransfer};
        furi_message_queue_put(event_queue, &event, 0);
    }

    if(LL_DMA_IsActiveFlag_TC1(DMA1)) {
        LL_DMA_ClearFlag_TC1(DMA1);

        DTMFDolphinCustomEvent event = {.type = DTMFDolphinEventDMAFullTransfer};
        furi_message_queue_put(event_queue, &event, 0);
    }
}

void dtmf_dolphin_audio_clear_samples(DTMFDolphinAudio* player) {
    for(size_t i = 0; i < player->buffer_length; i++) {
        player->sample_buffer[i] = 0;
    }
}

DTMFDolphinOsc* dtmf_dolphin_osc_alloc() {
    DTMFDolphinOsc* osc = malloc(sizeof(DTMFDolphinOsc));
    osc->cached_freq = 0;
    osc->offset = 0;
    osc->period = 0;
    osc->lookup_table = NULL;
    return osc;
}

DTMFDolphinPulseFilter* dtmf_dolphin_pulse_filter_alloc() {
    DTMFDolphinPulseFilter* pf = malloc(sizeof(DTMFDolphinPulseFilter));
    pf->duration = 0;
    pf->period = 0;
    pf->offset = 0;
    pf->lookup_table = NULL;
    return pf;
}

DTMFDolphinAudio* dtmf_dolphin_audio_alloc() {
    DTMFDolphinAudio* player = malloc(sizeof(DTMFDolphinAudio));
    player->buffer_length = SAMPLE_BUFFER_LENGTH;
    player->half_buffer_length = SAMPLE_BUFFER_LENGTH / 2;
    player->sample_buffer = malloc(sizeof(uint16_t) * player->buffer_length);
    player->osc1 = dtmf_dolphin_osc_alloc();
    player->osc2 = dtmf_dolphin_osc_alloc();
    player->volume = 1.0f;
    player->queue = furi_message_queue_alloc(10, sizeof(DTMFDolphinCustomEvent));
    player->filter = dtmf_dolphin_pulse_filter_alloc();
    player->playing = false;
    dtmf_dolphin_audio_clear_samples(player);

    return player;
}

size_t calc_waveform_period(float freq) {
    if(!freq) {
        return 0;
    }
    // DMA Rate calculation, thanks to Dr_Zlo
    float dma_rate = CPU_CLOCK_FREQ / 2 / DTMF_DOLPHIN_HAL_DMA_PRESCALER /
                     (DTMF_DOLPHIN_HAL_DMA_AUTORELOAD + 1);

    // Using a constant scaling modifier, which likely represents
    // the combined system overhead and isr latency.
    return (uint16_t)dma_rate * 2 / freq * 0.801923;
}

void osc_generate_lookup_table(DTMFDolphinOsc* osc, float freq) {
    if(osc->lookup_table != NULL) {
        free(osc->lookup_table);
    }
    osc->offset = 0;
    osc->cached_freq = freq;
    osc->period = calc_waveform_period(freq);
    if(!osc->period) {
        osc->lookup_table = NULL;
        return;
    }
    osc->lookup_table = malloc(sizeof(float) * osc->period);

    for(size_t i = 0; i < osc->period; i++) {
        osc->lookup_table[i] = sin(i * PERIOD_2_PI / osc->period) + 1;
    }
}

void filter_generate_lookup_table(
    DTMFDolphinPulseFilter* pf,
    uint16_t pulses,
    uint16_t pulse_ms,
    uint16_t gap_ms) {
    if(pf->lookup_table != NULL) {
        free(pf->lookup_table);
    }
    pf->offset = 0;

    uint16_t gap_period = calc_waveform_period(1000 / (float)gap_ms);
    uint16_t pulse_period = calc_waveform_period(1000 / (float)pulse_ms);
    pf->period = pulse_period + gap_period;

    if(!pf->period) {
        pf->lookup_table = NULL;
        return;
    }
    pf->duration = pf->period * pulses;
    pf->lookup_table = malloc(sizeof(bool) * pf->duration);

    for(size_t i = 0; i < pf->duration; i++) {
        pf->lookup_table[i] = i % pf->period < pulse_period;
    }
}

float sample_frame(DTMFDolphinOsc* osc) {
    float frame = 0.0;

    if(osc->period) {
        frame = osc->lookup_table[osc->offset];
        osc->offset = (osc->offset + 1) % osc->period;
    }

    return frame;
}

bool sample_filter(DTMFDolphinPulseFilter* pf) {
    bool frame = true;

    if(pf->duration) {
        if(pf->offset < pf->duration) {
            frame = pf->lookup_table[pf->offset];
            pf->offset = pf->offset + 1;
        } else {
            frame = false;
        }
    }

    return frame;
}

void dtmf_dolphin_osc_free(DTMFDolphinOsc* osc) {
    if(osc->lookup_table != NULL) {
        free(osc->lookup_table);
    }
    free(osc);
}

void dtmf_dolphin_filter_free(DTMFDolphinPulseFilter* pf) {
    if(pf->lookup_table != NULL) {
        free(pf->lookup_table);
    }
    free(pf);
}

void dtmf_dolphin_audio_free(DTMFDolphinAudio* player) {
    furi_message_queue_free(player->queue);
    dtmf_dolphin_osc_free(player->osc1);
    dtmf_dolphin_osc_free(player->osc2);
    dtmf_dolphin_filter_free(player->filter);
    free(player->sample_buffer);
    free(player);
    current_player = NULL;
}

bool generate_waveform(DTMFDolphinAudio* player, uint16_t buffer_index) {
    uint16_t* sample_buffer_start = &player->sample_buffer[buffer_index];

    for(size_t i = 0; i < player->half_buffer_length; i++) {
        float data = 0;
        if(player->osc2->period) {
            data = (sample_frame(player->osc1) / 2) + (sample_frame(player->osc2) / 2);
        } else {
            data = (sample_frame(player->osc1));
        }
        data *= sample_filter(player->filter) ? player->volume : 0.0;
        data *= UINT8_MAX / 2; // scale -128..127
        data += UINT8_MAX / 2; // to unsigned

        if(data < 0) {
            data = 0;
        }

        if(data > 255) {
            data = 255;
        }

        sample_buffer_start[i] = data;
    }

    return true;
}

bool dtmf_dolphin_audio_play_tones(
    float freq1,
    float freq2,
    uint16_t pulses,
    uint16_t pulse_ms,
    uint16_t gap_ms) {
    if(current_player != NULL && current_player->playing) {
        // Cannot start playing while still playing something else
        return false;
    }
    current_player = dtmf_dolphin_audio_alloc();

    osc_generate_lookup_table(current_player->osc1, freq1);
    osc_generate_lookup_table(current_player->osc2, freq2);
    filter_generate_lookup_table(current_player->filter, pulses, pulse_ms, gap_ms);

    generate_waveform(current_player, 0);
    generate_waveform(current_player, current_player->half_buffer_length);

    dtmf_dolphin_dma_init((uint32_t)current_player->sample_buffer, current_player->buffer_length);

    furi_hal_interrupt_set_isr(
        FuriHalInterruptIdDma1Ch1, dtmf_dolphin_audio_dma_isr, current_player->queue);
    if(furi_hal_speaker_acquire(1000)) {
        dtmf_dolphin_speaker_init();
        dtmf_dolphin_dma_start();
        dtmf_dolphin_speaker_start();
        current_player->playing = true;
        return true;
    } else {
        current_player->playing = false;
        return false;
    }
}

bool dtmf_dolphin_audio_stop_tones() {
    if(current_player != NULL && !current_player->playing) {
        // Can't stop a player that isn't playing.
        return false;
    }
    while(current_player->filter->offset > 0 &&
          current_player->filter->offset < current_player->filter->duration) {
        // run remaining ticks if needed to complete filter sequence
        dtmf_dolphin_audio_handle_tick();
    }
    dtmf_dolphin_speaker_stop();
    dtmf_dolphin_dma_stop();
    furi_hal_speaker_release();

    furi_hal_interrupt_set_isr(FuriHalInterruptIdDma1Ch1, NULL, NULL);

    dtmf_dolphin_audio_free(current_player);

    return true;
}

bool dtmf_dolphin_audio_handle_tick() {
    bool handled = false;

    if(current_player) {
        DTMFDolphinCustomEvent event;
        if(furi_message_queue_get(current_player->queue, &event, 250) == FuriStatusOk) {
            if(event.type == DTMFDolphinEventDMAHalfTransfer) {
                generate_waveform(current_player, 0);
                handled = true;
            } else if(event.type == DTMFDolphinEventDMAFullTransfer) {
                generate_waveform(current_player, current_player->half_buffer_length);
                handled = true;
            }
        }
    }
    return handled;
}