/* Copyright (C) 2023 Salvatore Sanfilippo -- All Rights Reserved
 * Copyright (C) 2023 Maciej Wojtasik -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"
#include <stream/stream.h>
#include <flipper_format/flipper_format_i.h>

/* ========================= Signal file operations ========================= */

/* This function saves the current logical signal on disk. What is saved here
 * is not the signal as level and duration as we received it from CC1101,
 * but it's logical representation stored in the app->msg_info bitmap, where
 * each 1 or 0 means a puls or gap for the specified short pulse duration time
 * (te). */
bool save_signal(ProtoViewApp* app, const char* filename) {
    /* We have a message at all? */
    if(app->msg_info == NULL || app->msg_info->pulses_count == 0) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    Stream* stream = flipper_format_get_raw_stream(file);
    FuriString* file_content = NULL;
    bool success = true;

    if(flipper_format_file_open_always(file, filename)) {
        /* Write the file header. */
        FuriString* file_content = furi_string_alloc();
        const char* preset_id = ProtoViewModulations[app->modulation].id;

        furi_string_printf(
            file_content,
            "Filetype: Flipper SubGhz RAW File\n"
            "Version: 1\n"
            "Frequency: %ld\n"
            "Preset: %s\n",
            app->frequency,
            preset_id ? preset_id : "FuriHalSubGhzPresetCustom");

        /* For custom modulations, we need to emit a set of registers. */
        if(preset_id == NULL) {
            FuriString* custom = furi_string_alloc();
            uint8_t* regs = ProtoViewModulations[app->modulation].custom;
            furi_string_printf(
                custom,
                "Custom_preset_module: CC1101\n"
                "Custom_preset_data: ");
            for(int j = 0; regs[j]; j += 2) {
                furi_string_cat_printf(custom, "%02X %02X ", (int)regs[j], (int)regs[j + 1]);
            }
            size_t len = furi_string_size(file_content);
            furi_string_set_char(custom, len - 1, '\n');
            furi_string_cat(file_content, custom);
            furi_string_free(custom);
        }

        /* We always save raw files. */
        furi_string_cat_printf(
            file_content,
            "Protocol: RAW\n"
            "RAW_Data: -10000\n"); // Start with 10 ms of gap

        /* Write header. */
        size_t len = furi_string_size(file_content);
        if(stream_write(stream, (uint8_t*)furi_string_get_cstr(file_content), len) != len) {
            FURI_LOG_W(TAG, "Short write to file");
            success = false;
            goto write_err;
        }
        furi_string_reset(file_content);

        /* Write raw data sections. The Flipper subghz parser can't handle
         * too much data on a single line, so we generate a new one
         * every few samples. */
        uint32_t this_line_samples = 0;
        uint32_t max_line_samples = 100;
        uint32_t idx = 0; // Iindex in the signal bitmap.
        ProtoViewMsgInfo* i = app->msg_info;
        while(idx < i->pulses_count) {
            bool level = bitmap_get(i->bits, i->bits_bytes, idx);
            uint32_t te_times = 1;
            idx++;
            /* Count the duration of the current pulse/gap. */
            while(idx < i->pulses_count && bitmap_get(i->bits, i->bits_bytes, idx) == level) {
                te_times++;
                idx++;
            }
            // Invariant: after the loop 'idx' is at the start of the
            // next gap or pulse.

            int32_t dur = (int32_t)i->short_pulse_dur * te_times;
            if(level == 0) dur = -dur; /* Negative is gap in raw files. */

            /* Emit the sample. If this is the first sample of the line,
             * also emit the RAW_Data: field. */
            if(this_line_samples == 0) furi_string_cat_printf(file_content, "RAW_Data: ");
            furi_string_cat_printf(file_content, "%d ", (int)dur);
            this_line_samples++;

            /* Store the current set of samples on disk, when we reach a
             * given number or the end of the signal. */
            bool end_reached = (idx == i->pulses_count);
            if(this_line_samples == max_line_samples || end_reached) {
                /* If that's the end, terminate the signal with a long
                 * gap. */
                if(end_reached) furi_string_cat_printf(file_content, "-10000 ");

                /* We always have a trailing space in the last sample. Make it
                 * a newline. */
                size_t len = furi_string_size(file_content);
                furi_string_set_char(file_content, len - 1, '\n');

                if(stream_write(stream, (uint8_t*)furi_string_get_cstr(file_content), len) !=
                   len) {
                    FURI_LOG_W(TAG, "Short write to file");
                    success = false;
                    goto write_err;
                }

                /* Prepare for next line. */
                furi_string_reset(file_content);
                this_line_samples = 0;
            }
        }
    } else {
        success = false;
        FURI_LOG_W(TAG, "Unable to open file");
    }

write_err:
    furi_record_close(RECORD_STORAGE);
    flipper_format_free(file);
    if(file_content != NULL) furi_string_free(file_content);
    return success;
}
