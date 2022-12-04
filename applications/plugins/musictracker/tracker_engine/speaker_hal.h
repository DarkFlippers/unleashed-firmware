#include <furi_hal.h>

void tracker_speaker_init();

void tracker_speaker_deinit();

void tracker_speaker_play(float frequency, float pwm);

void tracker_speaker_stop();

void tracker_interrupt_init(float freq, FuriHalInterruptISR isr, void* context);

void tracker_interrupt_deinit();

void tracker_debug_init();

void tracker_debug_set(bool value);

void tracker_debug_deinit();