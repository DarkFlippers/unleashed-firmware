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
#ifndef _CPU_H_
#define _CPU_H_

#include "hal.h"

#define MEMORY_SIZE 4096 // 4096 x 4 bits (640 x 4 bits of RAM)

#define MEM_RAM_ADDR 0x000
#define MEM_RAM_SIZE 0x280
#define MEM_DISPLAY1_ADDR 0xE00
#define MEM_DISPLAY1_SIZE 0x050
#define MEM_DISPLAY2_ADDR 0xE80
#define MEM_DISPLAY2_SIZE 0x050
#define MEM_IO_ADDR 0xF00
#define MEM_IO_SIZE 0x080

/* Define this if you want to reduce the footprint of the memory buffer from 4096 u4_t (most likely bytes)
 * to 464 u8_t (bytes for sure), while increasing slightly the number of operations needed to read/write from/to it.
 */
#define LOW_FOOTPRINT

#ifdef LOW_FOOTPRINT
/* Invalid memory areas are not buffered to reduce the footprint of the library in memory */
#define MEM_BUFFER_SIZE (MEM_RAM_SIZE + MEM_DISPLAY1_SIZE + MEM_DISPLAY2_SIZE + MEM_IO_SIZE) / 2

/* Maps the CPU memory to the memory buffer */
#define RAM_TO_MEMORY(n) ((n - MEM_RAM_ADDR) / 2)
#define DISP1_TO_MEMORY(n) ((n - MEM_DISPLAY1_ADDR + MEM_RAM_SIZE) / 2)
#define DISP2_TO_MEMORY(n) ((n - MEM_DISPLAY2_ADDR + MEM_RAM_SIZE + MEM_DISPLAY1_SIZE) / 2)
#define IO_TO_MEMORY(n) \
    ((n - MEM_IO_ADDR + MEM_RAM_SIZE + MEM_DISPLAY1_SIZE + MEM_DISPLAY2_SIZE) / 2)

#define SET_RAM_MEMORY(buffer, n, v)                                                         \
    {                                                                                        \
        buffer[RAM_TO_MEMORY(n)] = (buffer[RAM_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | \
                                   ((v)&0xF) << (((n) % 2) << 2);                            \
    }
#define SET_DISP1_MEMORY(buffer, n, v)                                                           \
    {                                                                                            \
        buffer[DISP1_TO_MEMORY(n)] = (buffer[DISP1_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | \
                                     ((v)&0xF) << (((n) % 2) << 2);                              \
    }
#define SET_DISP2_MEMORY(buffer, n, v)                                                           \
    {                                                                                            \
        buffer[DISP2_TO_MEMORY(n)] = (buffer[DISP2_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | \
                                     ((v)&0xF) << (((n) % 2) << 2);                              \
    }
#define SET_IO_MEMORY(buffer, n, v)                                                        \
    {                                                                                      \
        buffer[IO_TO_MEMORY(n)] = (buffer[IO_TO_MEMORY(n)] & ~(0xF << (((n) % 2) << 2))) | \
                                  ((v)&0xF) << (((n) % 2) << 2);                           \
    }
#define SET_MEMORY(buffer, n, v)                                   \
    {                                                              \
        if((n) < (MEM_RAM_ADDR + MEM_RAM_SIZE)) {                  \
            SET_RAM_MEMORY(buffer, n, v);                          \
        } else if((n) < MEM_DISPLAY1_ADDR) {                       \
            /* INVALID_MEMORY */                                   \
        } else if((n) < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) { \
            SET_DISP1_MEMORY(buffer, n, v);                        \
        } else if((n) < MEM_DISPLAY2_ADDR) {                       \
            /* INVALID_MEMORY */                                   \
        } else if((n) < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) { \
            SET_DISP2_MEMORY(buffer, n, v);                        \
        } else if((n) < MEM_IO_ADDR) {                             \
            /* INVALID_MEMORY */                                   \
        } else if((n) < (MEM_IO_ADDR + MEM_IO_SIZE)) {             \
            SET_IO_MEMORY(buffer, n, v);                           \
        } else {                                                   \
            /* INVALID_MEMORY */                                   \
        }                                                          \
    }

#define GET_RAM_MEMORY(buffer, n) ((buffer[RAM_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_DISP1_MEMORY(buffer, n) ((buffer[DISP1_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_DISP2_MEMORY(buffer, n) ((buffer[DISP2_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_IO_MEMORY(buffer, n) ((buffer[IO_TO_MEMORY(n)] >> (((n) % 2) << 2)) & 0xF)
#define GET_MEMORY(buffer, n)                                                     \
    ((buffer                                                                      \
          [((n) < (MEM_RAM_ADDR + MEM_RAM_SIZE))           ? RAM_TO_MEMORY(n) :   \
           ((n) < MEM_DISPLAY1_ADDR)                       ? 0 :                  \
           ((n) < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) ? DISP1_TO_MEMORY(n) : \
           ((n) < MEM_DISPLAY2_ADDR)                       ? 0 :                  \
           ((n) < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) ? DISP2_TO_MEMORY(n) : \
           ((n) < MEM_IO_ADDR)                             ? 0 :                  \
           ((n) < (MEM_IO_ADDR + MEM_IO_SIZE))             ? IO_TO_MEMORY(n) :    \
                                                             0] >>                            \
      (((n) % 2) << 2)) &                                                         \
     0xF)

#define MEM_BUFFER_TYPE u8_t
#else
#define MEM_BUFFER_SIZE MEMORY_SIZE

#define SET_MEMORY(buffer, n, v) \
    { buffer[n] = v; }
#define SET_RAM_MEMORY(buffer, n, v) SET_MEMORY(buffer, n, v)
#define SET_DISP1_MEMORY(buffer, n, v) SET_MEMORY(buffer, n, v)
#define SET_DISP2_MEMORY(buffer, n, v) SET_MEMORY(buffer, n, v)
#define SET_IO_MEMORY(buffer, n, v) SET_MEMORY(buffer, n, v)

#define GET_MEMORY(buffer, n) (buffer[n])
#define GET_RAM_MEMORY(buffer, n) GET_MEMORY(buffer, n)
#define GET_DISP1_MEMORY(buffer, n) GET_MEMORY(buffer, n)
#define GET_DISP2_MEMORY(buffer, n) GET_MEMORY(buffer, n)
#define GET_IO_MEMORY(buffer, n) GET_MEMORY(buffer, n)

#define MEM_BUFFER_TYPE u4_t
#endif

typedef struct breakpoint {
    u13_t addr;
    struct breakpoint* next;
} breakpoint_t;

/* Pins (TODO: add other pins) */
typedef enum {
    PIN_K00 = 0x0,
    PIN_K01 = 0x1,
    PIN_K02 = 0x2,
    PIN_K03 = 0x3,
    PIN_K10 = 0X4,
    PIN_K11 = 0X5,
    PIN_K12 = 0X6,
    PIN_K13 = 0X7,
} pin_t;

typedef enum {
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH = 1,
} pin_state_t;

typedef enum {
    INT_PROG_TIMER_SLOT = 0,
    INT_SERIAL_SLOT = 1,
    INT_K10_K13_SLOT = 2,
    INT_K00_K03_SLOT = 3,
    INT_STOPWATCH_SLOT = 4,
    INT_CLOCK_TIMER_SLOT = 5,
    INT_SLOT_NUM,
} int_slot_t;

typedef struct {
    u4_t factor_flag_reg;
    u4_t mask_reg;
    bool_t triggered; /* 1 if triggered, 0 otherwise */
    u8_t vector;
} interrupt_t;

typedef struct {
    u13_t* pc;
    u12_t* x;
    u12_t* y;
    u4_t* a;
    u4_t* b;
    u5_t* np;
    u8_t* sp;
    u4_t* flags;

    u32_t* tick_counter;
    u32_t* clk_timer_timestamp;
    u32_t* prog_timer_timestamp;
    bool_t* prog_timer_enabled;
    u8_t* prog_timer_data;
    u8_t* prog_timer_rld;

    u32_t* call_depth;

    interrupt_t* interrupts;

    MEM_BUFFER_TYPE* memory;
} state_t;

void cpu_add_bp(breakpoint_t** list, u13_t addr);
void cpu_free_bp(breakpoint_t** list);

void cpu_set_speed(u8_t speed);

state_t* cpu_get_state(void);

u32_t cpu_get_depth(void);

void cpu_set_input_pin(pin_t pin, pin_state_t state);

void cpu_sync_ref_timestamp(void);

void cpu_refresh_hw(void);

void cpu_reset(void);

bool_t cpu_init(const u12_t* program, breakpoint_t* breakpoints, u32_t freq);
void cpu_release(void);

int cpu_step(void);

#endif /* _CPU_H_ */
