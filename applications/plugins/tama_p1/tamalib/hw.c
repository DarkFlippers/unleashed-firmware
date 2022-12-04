/*
 * TamaLIB - A hardware agnostic Tamagotchi P1 emulation library
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "hw.h"
#include "cpu.h"
#include "hal.h"

/* SEG -> LCD mapping */
static u8_t seg_pos[40] = {0,  1,  2,  3,  4,  5,  6,  7,  32, 8,  9,  10, 11, 12,
                           13, 14, 15, 33, 34, 35, 31, 30, 29, 28, 27, 26, 25, 24,
                           36, 23, 22, 21, 20, 19, 18, 17, 16, 37, 38, 39};

bool_t hw_init(void) {
    /* Buttons are active LOW */
    cpu_set_input_pin(PIN_K00, PIN_STATE_HIGH);
    cpu_set_input_pin(PIN_K01, PIN_STATE_HIGH);
    cpu_set_input_pin(PIN_K02, PIN_STATE_HIGH);

    return 0;
}

void hw_release(void) {
}

void hw_set_lcd_pin(u8_t seg, u8_t com, u8_t val) {
    if(seg_pos[seg] < LCD_WIDTH) {
        g_hal->set_lcd_matrix(seg_pos[seg], com, val);
    } else {
        /*
		 * IC n -> seg-com|...
		 * IC 0 ->  8-0 |18-3 |19-2
		 * IC 1 ->  8-1 |17-0 |19-3
		 * IC 2 ->  8-2 |17-1 |37-12|38-13|39-14
		 * IC 3 ->  8-3 |17-2 |18-1 |19-0
		 * IC 4 -> 28-12|37-13|38-14|39-15
		 * IC 5 -> 28-13|37-14|38-15
		 * IC 6 -> 28-14|37-15|39-12
		 * IC 7 -> 28-15|38-12|39-13
		 */
        if(seg == 8 && com < 4) {
            g_hal->set_lcd_icon(com, val);
        } else if(seg == 28 && com >= 12) {
            g_hal->set_lcd_icon(com - 8, val);
        }
    }
}

void hw_set_button(button_t btn, btn_state_t state) {
    pin_state_t pin_state = (state == BTN_STATE_PRESSED) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    switch(btn) {
    case BTN_LEFT:
        cpu_set_input_pin(PIN_K02, pin_state);
        break;

    case BTN_MIDDLE:
        cpu_set_input_pin(PIN_K01, pin_state);
        break;

    case BTN_RIGHT:
        cpu_set_input_pin(PIN_K00, pin_state);
        break;
    }
}

void hw_set_buzzer_freq(u4_t freq) {
    u32_t snd_freq = 0;

    switch(freq) {
    case 0:
        /* 4096.0 Hz */
        snd_freq = 40960;
        break;

    case 1:
        /* 3276.8 Hz */
        snd_freq = 32768;
        break;

    case 2:
        /* 2730.7 Hz */
        snd_freq = 27307;
        break;

    case 3:
        /* 2340.6 Hz */
        snd_freq = 23406;
        break;

    case 4:
        /* 2048.0 Hz */
        snd_freq = 20480;
        break;

    case 5:
        /* 1638.4 Hz */
        snd_freq = 16384;
        break;

    case 6:
        /* 1365.3 Hz */
        snd_freq = 13653;
        break;

    case 7:
        /* 1170.3 Hz */
        snd_freq = 11703;
        break;
    }

    if(snd_freq != 0) {
        g_hal->set_frequency(snd_freq);
    }
}

void hw_enable_buzzer(bool_t en) {
    g_hal->play_frequency(en);
}
