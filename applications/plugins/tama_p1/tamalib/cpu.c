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
#include "cpu.h"
#include "hw.h"
#include "hal.h"

#define TICK_FREQUENCY 32768 // Hz

#define TIMER_1HZ_PERIOD 32768 // in ticks
#define TIMER_256HZ_PERIOD 128 // in ticks

#define MASK_4B 0xF00
#define MASK_6B 0xFC0
#define MASK_7B 0xFE0
#define MASK_8B 0xFF0
#define MASK_10B 0xFFC
#define MASK_12B 0xFFF

#define PCS (pc & 0xFF)
#define PCSL (pc & 0xF)
#define PCSH ((pc >> 4) & 0xF)
#define PCP ((pc >> 8) & 0xF)
#define PCB ((pc >> 12) & 0x1)
#define TO_PC(bank, page, step) ((step & 0xFF) | ((page & 0xF) << 8) | (bank & 0x1) << 12)
#define NBP ((np >> 4) & 0x1)
#define NPP (np & 0xF)
#define TO_NP(bank, page) ((page & 0xF) | (bank & 0x1) << 4)
#define XHL (x & 0xFF)
#define XL (x & 0xF)
#define XH ((x >> 4) & 0xF)
#define XP ((x >> 8) & 0xF)
#define YHL (y & 0xFF)
#define YL (y & 0xF)
#define YH ((y >> 4) & 0xF)
#define YP ((y >> 8) & 0xF)
#define M(n) get_memory(n)
#define SET_M(n, v) set_memory(n, v)
#define RQ(i) get_rq(i)
#define SET_RQ(i, v) set_rq(i, v)
#define SPL (sp & 0xF)
#define SPH ((sp >> 4) & 0xF)

#define FLAG_C (0x1 << 0)
#define FLAG_Z (0x1 << 1)
#define FLAG_D (0x1 << 2)
#define FLAG_I (0x1 << 3)

#define C !!(flags & FLAG_C)
#define Z !!(flags & FLAG_Z)
#define D !!(flags & FLAG_D)
#define I !!(flags & FLAG_I)

#define SET_C() \
    { flags |= FLAG_C; }
#define CLEAR_C() \
    { flags &= ~FLAG_C; }
#define SET_Z() \
    { flags |= FLAG_Z; }
#define CLEAR_Z() \
    { flags &= ~FLAG_Z; }
#define SET_D() \
    { flags |= FLAG_D; }
#define CLEAR_D() \
    { flags &= ~FLAG_D; }
#define SET_I() \
    { flags |= FLAG_I; }
#define CLEAR_I() \
    { flags &= ~FLAG_I; }

#define REG_CLK_INT_FACTOR_FLAGS 0xF00
#define REG_SW_INT_FACTOR_FLAGS 0xF01
#define REG_PROG_INT_FACTOR_FLAGS 0xF02
#define REG_SERIAL_INT_FACTOR_FLAGS 0xF03
#define REG_K00_K03_INT_FACTOR_FLAGS 0xF04
#define REG_K10_K13_INT_FACTOR_FLAGS 0xF05
#define REG_CLOCK_INT_MASKS 0xF10
#define REG_SW_INT_MASKS 0xF11
#define REG_PROG_INT_MASKS 0xF12
#define REG_SERIAL_INT_MASKS 0xF13
#define REG_K00_K03_INT_MASKS 0xF14
#define REG_K10_K13_INT_MASKS 0xF15
#define REG_PROG_TIMER_DATA_L 0xF24
#define REG_PROG_TIMER_DATA_H 0xF25
#define REG_PROG_TIMER_RELOAD_DATA_L 0xF26
#define REG_PROG_TIMER_RELOAD_DATA_H 0xF27
#define REG_K00_K03_INPUT_PORT 0xF40
#define REG_K10_K13_INPUT_PORT 0xF42
#define REG_K40_K43_BZ_OUTPUT_PORT 0xF54
#define REG_CPU_OSC3_CTRL 0xF70
#define REG_LCD_CTRL 0xF71
#define REG_LCD_CONTRAST 0xF72
#define REG_SVD_CTRL 0xF73
#define REG_BUZZER_CTRL1 0xF74
#define REG_BUZZER_CTRL2 0xF75
#define REG_CLK_WD_TIMER_CTRL 0xF76
#define REG_SW_TIMER_CTRL 0xF77
#define REG_PROG_TIMER_CTRL 0xF78
#define REG_PROG_TIMER_CLK_SEL 0xF79

#define INPUT_PORT_NUM 2

typedef struct {
    char* log;
    u12_t code;
    u12_t mask;
    u12_t shift_arg0;
    u12_t mask_arg0; // != 0 only if there are two arguments
    u8_t cycles;
    void (*cb)(u8_t arg0, u8_t arg1);
} op_t;

typedef struct {
    u4_t states;
} input_port_t;

/* Registers */
static u13_t pc, next_pc;
static u12_t x, y;
static u4_t a, b;
static u5_t np;
static u8_t sp;

/* Flags */
static u4_t flags;

static const u12_t* g_program = NULL;
static MEM_BUFFER_TYPE memory[MEM_BUFFER_SIZE];

static input_port_t inputs[INPUT_PORT_NUM] = {{0}};

/* Interrupts (in priority order) */
static interrupt_t interrupts[INT_SLOT_NUM] = {
    {0x0, 0x0, 0, 0x0C}, // Prog timer
    {0x0, 0x0, 0, 0x0A}, // Serial interface
    {0x0, 0x0, 0, 0x08}, // Input (K10-K13)
    {0x0, 0x0, 0, 0x06}, // Input (K00-K03)
    {0x0, 0x0, 0, 0x04}, // Stopwatch timer
    {0x0, 0x0, 0, 0x02}, // Clock timer
};

static breakpoint_t* g_breakpoints = NULL;

static u32_t call_depth = 0;

static u32_t clk_timer_timestamp = 0; // in ticks
static u32_t prog_timer_timestamp = 0; // in ticks
static bool_t prog_timer_enabled = 0;
static u8_t prog_timer_data = 0;
static u8_t prog_timer_rld = 0;

static u32_t tick_counter = 0;
static u32_t ts_freq;
static u8_t speed_ratio = 1;
static timestamp_t ref_ts;

static state_t cpu_state = {
    .pc = &pc,
    .x = &x,
    .y = &y,
    .a = &a,
    .b = &b,
    .np = &np,
    .sp = &sp,
    .flags = &flags,

    .tick_counter = &tick_counter,
    .clk_timer_timestamp = &clk_timer_timestamp,
    .prog_timer_timestamp = &prog_timer_timestamp,
    .prog_timer_enabled = &prog_timer_enabled,
    .prog_timer_data = &prog_timer_data,
    .prog_timer_rld = &prog_timer_rld,

    .call_depth = &call_depth,

    .interrupts = interrupts,

    .memory = memory,
};

void cpu_add_bp(breakpoint_t** list, u13_t addr) {
    breakpoint_t* bp;

    bp = (breakpoint_t*)g_hal->malloc(sizeof(breakpoint_t));
    if(!bp) {
        g_hal->log(LOG_ERROR, "Cannot allocate memory for breakpoint 0x%04X!\n", addr);
        return;
    }

    bp->addr = addr;

    if(*list != NULL) {
        bp->next = *list;
    } else {
        /* List is empty */
        bp->next = NULL;
    }

    *list = bp;
}

void cpu_free_bp(breakpoint_t** list) {
    breakpoint_t *bp = *list, *tmp;

    while(bp != NULL) {
        tmp = bp->next;
        g_hal->free(bp);
        bp = tmp;
    }

    *list = NULL;
}

void cpu_set_speed(u8_t speed) {
    speed_ratio = speed;
}

state_t* cpu_get_state(void) {
    return &cpu_state;
}

u32_t cpu_get_depth(void) {
    return call_depth;
}

static void generate_interrupt(int_slot_t slot, u8_t bit) {
    /* Set the factor flag no matter what */
    interrupts[slot].factor_flag_reg = interrupts[slot].factor_flag_reg | (0x1 << bit);

    /* Trigger the INT only if not masked */
    if(interrupts[slot].mask_reg & (0x1 << bit)) {
        interrupts[slot].triggered = 1;
    }
}

void cpu_set_input_pin(pin_t pin, pin_state_t state) {
    /* Set the I/O */
    inputs[pin & 0x4].states = (inputs[pin & 0x4].states & ~(0x1 << (pin & 0x3))) |
                               (state << (pin & 0x3));

    /* Trigger the interrupt (TODO: handle relation register) */
    if(state == PIN_STATE_LOW) {
        switch((pin & 0x4) >> 2) {
        case 0:
            generate_interrupt(INT_K00_K03_SLOT, pin & 0x3);
            break;

        case 1:
            generate_interrupt(INT_K10_K13_SLOT, pin & 0x3);
            break;
        }
    }
}

void cpu_sync_ref_timestamp(void) {
    ref_ts = g_hal->get_timestamp();
}

static u4_t get_io(u12_t n) {
    u4_t tmp;

    switch(n) {
    case REG_CLK_INT_FACTOR_FLAGS:
        /* Interrupt factor flags (clock timer) */
        tmp = interrupts[INT_CLOCK_TIMER_SLOT].factor_flag_reg;
        interrupts[INT_CLOCK_TIMER_SLOT].factor_flag_reg = 0;
        return tmp;

    case REG_SW_INT_FACTOR_FLAGS:
        /* Interrupt factor flags (stopwatch) */
        tmp = interrupts[INT_STOPWATCH_SLOT].factor_flag_reg;
        interrupts[INT_STOPWATCH_SLOT].factor_flag_reg = 0;
        return tmp;

    case REG_PROG_INT_FACTOR_FLAGS:
        /* Interrupt factor flags (prog timer) */
        tmp = interrupts[INT_PROG_TIMER_SLOT].factor_flag_reg;
        interrupts[INT_PROG_TIMER_SLOT].factor_flag_reg = 0;
        return tmp;

    case REG_SERIAL_INT_FACTOR_FLAGS:
        /* Interrupt factor flags (serial) */
        tmp = interrupts[INT_SERIAL_SLOT].factor_flag_reg;
        interrupts[INT_SERIAL_SLOT].factor_flag_reg = 0;
        return tmp;

    case REG_K00_K03_INT_FACTOR_FLAGS:
        /* Interrupt factor flags (K00-K03) */
        tmp = interrupts[INT_K00_K03_SLOT].factor_flag_reg;
        interrupts[INT_K00_K03_SLOT].factor_flag_reg = 0;
        return tmp;

    case REG_K10_K13_INT_FACTOR_FLAGS:
        /* Interrupt factor flags (K10-K13) */
        tmp = interrupts[INT_K10_K13_SLOT].factor_flag_reg;
        interrupts[INT_K10_K13_SLOT].factor_flag_reg = 0;
        return tmp;

    case REG_CLOCK_INT_MASKS:
        /* Clock timer interrupt masks */
        return interrupts[INT_CLOCK_TIMER_SLOT].mask_reg;

    case REG_SW_INT_MASKS:
        /* Stopwatch interrupt masks */
        return interrupts[INT_STOPWATCH_SLOT].mask_reg & 0x3;

    case REG_PROG_INT_MASKS:
        /* Prog timer interrupt masks */
        return interrupts[INT_PROG_TIMER_SLOT].mask_reg & 0x1;

    case REG_SERIAL_INT_MASKS:
        /* Serial interface interrupt masks */
        return interrupts[INT_SERIAL_SLOT].mask_reg & 0x1;

    case REG_K00_K03_INT_MASKS:
        /* Input (K00-K03) interrupt masks */
        return interrupts[INT_K00_K03_SLOT].mask_reg;

    case REG_K10_K13_INT_MASKS:
        /* Input (K10-K13) interrupt masks */
        return interrupts[INT_K10_K13_SLOT].mask_reg;

    case REG_PROG_TIMER_DATA_L:
        /* Prog timer data (low) */
        return prog_timer_data & 0xF;

    case REG_PROG_TIMER_DATA_H:
        /* Prog timer data (high) */
        return (prog_timer_data >> 4) & 0xF;

    case REG_PROG_TIMER_RELOAD_DATA_L:
        /* Prog timer reload data (low) */
        return prog_timer_rld & 0xF;

    case REG_PROG_TIMER_RELOAD_DATA_H:
        /* Prog timer reload data (high) */
        return (prog_timer_rld >> 4) & 0xF;

    case REG_K00_K03_INPUT_PORT:
        /* Input port (K00-K03) */
        return inputs[0].states;

    case REG_K10_K13_INPUT_PORT:
        /* Input port (K10-K13) */
        return inputs[1].states;

    case REG_K40_K43_BZ_OUTPUT_PORT:
        /* Output port (R40-R43) */
        return GET_IO_MEMORY(memory, n);

    case REG_CPU_OSC3_CTRL:
        /* CPU/OSC3 clocks switch, CPU voltage switch */
        return GET_IO_MEMORY(memory, n);

    case REG_LCD_CTRL:
        /* LCD control */
        return GET_IO_MEMORY(memory, n);

    case REG_LCD_CONTRAST:
        /* LCD contrast */
        break;

    case REG_SVD_CTRL:
        /* SVD */
        return GET_IO_MEMORY(memory, n) & 0x7; // Voltage always OK

    case REG_BUZZER_CTRL1:
        /* Buzzer config 1 */
        return GET_IO_MEMORY(memory, n);

    case REG_BUZZER_CTRL2:
        /* Buzzer config 2 */
        return GET_IO_MEMORY(memory, n) & 0x3; // Buzzer ready

    case REG_CLK_WD_TIMER_CTRL:
        /* Clock/Watchdog timer reset */
        break;

    case REG_SW_TIMER_CTRL:
        /* Stopwatch stop/run/reset */
        break;

    case REG_PROG_TIMER_CTRL:
        /* Prog timer stop/run/reset */
        return !!prog_timer_enabled;

    case REG_PROG_TIMER_CLK_SEL:
        /* Prog timer clock selection */
        break;

    default:
        g_hal->log(LOG_ERROR, "Read from unimplemented I/O 0x%03X - PC = 0x%04X\n", n, pc);
    }

    return 0;
}

static void set_io(u12_t n, u4_t v) {
    switch(n) {
    case REG_CLOCK_INT_MASKS:
        /* Clock timer interrupt masks */
        /* Assume 1Hz timer INT enabled (0x8) */
        interrupts[INT_CLOCK_TIMER_SLOT].mask_reg = v;
        break;

    case REG_SW_INT_MASKS:
        /* Stopwatch interrupt masks */
        /* Assume all INT disabled */
        interrupts[INT_STOPWATCH_SLOT].mask_reg = v;
        break;

    case REG_PROG_INT_MASKS:
        /* Prog timer interrupt masks */
        /* Assume Prog timer INT enabled (0x1) */
        interrupts[INT_PROG_TIMER_SLOT].mask_reg = v;
        break;

    case REG_SERIAL_INT_MASKS:
        /* Serial interface interrupt masks */
        /* Assume all INT disabled */
        interrupts[INT_K10_K13_SLOT].mask_reg = v;
        break;

    case REG_K00_K03_INT_MASKS:
        /* Input (K00-K03) interrupt masks */
        /* Assume all INT disabled */
        interrupts[INT_SERIAL_SLOT].mask_reg = v;
        break;

    case REG_K10_K13_INT_MASKS:
        /* Input (K10-K13) interrupt masks */
        /* Assume all INT disabled */
        interrupts[INT_K10_K13_SLOT].mask_reg = v;
        break;

    case REG_PROG_TIMER_RELOAD_DATA_L:
        /* Prog timer reload data (low) */
        prog_timer_rld = v | (prog_timer_rld & 0xF0);
        break;

    case REG_PROG_TIMER_RELOAD_DATA_H:
        /* Prog timer reload data (high) */
        prog_timer_rld = (prog_timer_rld & 0xF) | (v << 4);
        break;

    case REG_K00_K03_INPUT_PORT:
        /* Input port (K00-K03) */
        /* Write not allowed */
        break;

    case REG_K40_K43_BZ_OUTPUT_PORT:
        /* Output port (R40-R43) */
        //g_hal->log(LOG_INFO, "Output/Buzzer: 0x%X\n", v);
        hw_enable_buzzer(!(v & 0x8));
        break;

    case REG_CPU_OSC3_CTRL:
        /* CPU/OSC3 clocks switch, CPU voltage switch */
        /* Assume 32,768 OSC1 selected, OSC3 off, battery >= 3,1V (0x1) */
        break;

    case REG_LCD_CTRL:
        /* LCD control */
        break;

    case REG_LCD_CONTRAST:
        /* LCD contrast */
        /* Assume medium contrast (0x8) */
        break;

    case REG_SVD_CTRL:
        /* SVD */
        /* Assume battery voltage always OK (0x6) */
        break;

    case REG_BUZZER_CTRL1:
        /* Buzzer config 1 */
        hw_set_buzzer_freq(v & 0x7);
        break;

    case REG_BUZZER_CTRL2:
        /* Buzzer config 2 */
        break;

    case REG_CLK_WD_TIMER_CTRL:
        /* Clock/Watchdog timer reset */
        /* Ignore watchdog */
        break;

    case REG_SW_TIMER_CTRL:
        /* Stopwatch stop/run/reset */
        break;

    case REG_PROG_TIMER_CTRL:
        /* Prog timer stop/run/reset */
        if(v & 0x2) {
            prog_timer_data = prog_timer_rld;
        }

        if((v & 0x1) && !prog_timer_enabled) {
            prog_timer_timestamp = tick_counter;
        }

        prog_timer_enabled = v & 0x1;
        break;

    case REG_PROG_TIMER_CLK_SEL:
        /* Prog timer clock selection */
        /* Assume 256Hz, output disabled */
        break;

    default:
        g_hal->log(LOG_ERROR, "Write 0x%X to unimplemented I/O 0x%03X - PC = 0x%04X\n", v, n, pc);
    }
}

static void set_lcd(u12_t n, u4_t v) {
    u8_t i;
    u8_t seg, com0;

    seg = ((n & 0x7F) >> 1);
    com0 = (((n & 0x80) >> 7) * 8 + (n & 0x1) * 4);

    for(i = 0; i < 4; i++) {
        hw_set_lcd_pin(seg, com0 + i, (v >> i) & 0x1);
    }
}

static u4_t get_memory(u12_t n) {
    u4_t res = 0;

    if(n < MEM_RAM_SIZE) {
        /* RAM */
        g_hal->log(LOG_MEMORY, "RAM              - ");
        res = GET_RAM_MEMORY(memory, n);
    } else if(n >= MEM_DISPLAY1_ADDR && n < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {
        /* Display Memory 1 */
        g_hal->log(LOG_MEMORY, "Display Memory 1 - ");
        res = GET_DISP1_MEMORY(memory, n);
    } else if(n >= MEM_DISPLAY2_ADDR && n < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {
        /* Display Memory 2 */
        g_hal->log(LOG_MEMORY, "Display Memory 2 - ");
        res = GET_DISP2_MEMORY(memory, n);
    } else if(n >= MEM_IO_ADDR && n < (MEM_IO_ADDR + MEM_IO_SIZE)) {
        /* I/O Memory */
        g_hal->log(LOG_MEMORY, "I/O              - ");
        res = get_io(n);
    } else {
        g_hal->log(LOG_ERROR, "Read from invalid memory address 0x%03X - PC = 0x%04X\n", n, pc);
        return 0;
    }

    g_hal->log(LOG_MEMORY, "Read  0x%X - Address 0x%03X - PC = 0x%04X\n", res, n, pc);

    return res;
}

static void set_memory(u12_t n, u4_t v) {
    /* Cache any data written to a valid address, and process it */
    if(n < MEM_RAM_SIZE) {
        /* RAM */
        SET_RAM_MEMORY(memory, n, v);
        g_hal->log(LOG_MEMORY, "RAM              - ");
    } else if(n >= MEM_DISPLAY1_ADDR && n < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {
        /* Display Memory 1 */
        SET_DISP1_MEMORY(memory, n, v);
        set_lcd(n, v);
        g_hal->log(LOG_MEMORY, "Display Memory 1 - ");
    } else if(n >= MEM_DISPLAY2_ADDR && n < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {
        /* Display Memory 2 */
        SET_DISP2_MEMORY(memory, n, v);
        set_lcd(n, v);
        g_hal->log(LOG_MEMORY, "Display Memory 2 - ");
    } else if(n >= MEM_IO_ADDR && n < (MEM_IO_ADDR + MEM_IO_SIZE)) {
        /* I/O Memory */
        SET_IO_MEMORY(memory, n, v);
        set_io(n, v);
        g_hal->log(LOG_MEMORY, "I/O              - ");
    } else {
        g_hal->log(
            LOG_ERROR, "Write 0x%X to invalid memory address 0x%03X - PC = 0x%04X\n", v, n, pc);
        return;
    }

    g_hal->log(LOG_MEMORY, "Write 0x%X - Address 0x%03X - PC = 0x%04X\n", v, n, pc);
}

void cpu_refresh_hw(void) {
    static const struct range {
        u12_t addr;
        u12_t size;
    } refresh_locs[] = {
        {MEM_DISPLAY1_ADDR, MEM_DISPLAY1_SIZE}, /* Display Memory 1 */
        {MEM_DISPLAY2_ADDR, MEM_DISPLAY2_SIZE}, /* Display Memory 2 */
        {REG_BUZZER_CTRL1, 1}, /* Buzzer frequency */
        {REG_K40_K43_BZ_OUTPUT_PORT, 1}, /* Buzzer enabled */

        {0, 0}, // end of list
    };

    for(int i = 0; refresh_locs[i].size != 0; i++) {
        for(u12_t n = refresh_locs[i].addr; n < (refresh_locs[i].addr + refresh_locs[i].size);
            n++) {
            set_memory(n, GET_MEMORY(memory, n));
        }
    }
}

static u4_t get_rq(u12_t rq) {
    switch(rq & 0x3) {
    case 0x0:
        return a;

    case 0x1:
        return b;

    case 0x2:
        return M(x);

    case 0x3:
        return M(y);
    }

    return 0;
}

static void set_rq(u12_t rq, u4_t v) {
    switch(rq & 0x3) {
    case 0x0:
        a = v;
        break;

    case 0x1:
        b = v;
        break;

    case 0x2:
        SET_M(x, v);
        break;

    case 0x3:
        SET_M(y, v);
        break;
    }
}

/* Instructions */
static void op_pset_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    np = arg0;
}

static void op_jp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    next_pc = arg0 | (np << 8);
}

static void op_jp_c_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(flags & FLAG_C) {
        next_pc = arg0 | (np << 8);
    }
}

static void op_jp_nc_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(!(flags & FLAG_C)) {
        next_pc = arg0 | (np << 8);
    }
}

static void op_jp_z_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(flags & FLAG_Z) {
        next_pc = arg0 | (np << 8);
    }
}

static void op_jp_nz_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(!(flags & FLAG_Z)) {
        next_pc = arg0 | (np << 8);
    }
}

static void op_jpba_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    next_pc = a | (b << 4) | (np << 8);
}

static void op_call_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    pc = (pc + 1) & 0x1FFF; // This does not actually change the PC register
    SET_M(sp - 1, PCP);
    SET_M(sp - 2, PCSH);
    SET_M(sp - 3, PCSL);
    sp = (sp - 3) & 0xFF;
    next_pc = TO_PC(PCB, NPP, arg0);
    call_depth++;
}

static void op_calz_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    pc = (pc + 1) & 0x1FFF; // This does not actually change the PC register
    SET_M(sp - 1, PCP);
    SET_M(sp - 2, PCSH);
    SET_M(sp - 3, PCSL);
    sp = (sp - 3) & 0xFF;
    next_pc = TO_PC(PCB, 0, arg0);
    call_depth++;
}

static void op_ret_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    next_pc = M(sp) | (M(sp + 1) << 4) | (M(sp + 2) << 8) | (PCB << 12);
    sp = (sp + 3) & 0xFF;
    call_depth--;
}

static void op_rets_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    next_pc = M(sp) | (M(sp + 1) << 4) | (M(sp + 2) << 8) | (PCB << 12);
    sp = (sp + 3) & 0xFF;
    next_pc = (pc + 1) & 0x1FFF;
    call_depth--;
}

static void op_retd_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    next_pc = M(sp) | (M(sp + 1) << 4) | (M(sp + 2) << 8) | (PCB << 12);
    sp = (sp + 3) & 0xFF;
    SET_M(x, arg0 & 0xF);
    SET_M(x + 1, (arg0 >> 4) & 0xF);
    x = ((x + 2) & 0xFF) | (XP << 8);
    call_depth--;
}

static void op_nop5_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
}

static void op_nop7_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
}

static void op_halt_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    g_hal->halt();
}

static void op_inc_x_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_inc_y_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_ld_x_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    x = arg0 | (XP << 8);
}

static void op_ld_y_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    y = arg0 | (YP << 8);
}

static void op_ld_xp_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    x = XHL | (RQ(arg0) << 8);
}

static void op_ld_xh_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    x = XL | (RQ(arg0) << 4) | (XP << 8);
}

static void op_ld_xl_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    x = RQ(arg0) | (XH << 4) | (XP << 8);
}

static void op_ld_yp_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    y = YHL | (RQ(arg0) << 8);
}

static void op_ld_yh_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    y = YL | (RQ(arg0) << 4) | (YP << 8);
}

static void op_ld_yl_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    y = RQ(arg0) | (YH << 4) | (YP << 8);
}

static void op_ld_r_xp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, XP);
}

static void op_ld_r_xh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, XH);
}

static void op_ld_r_xl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, XL);
}

static void op_ld_r_yp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, YP);
}

static void op_ld_r_yh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, YH);
}

static void op_ld_r_yl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, YL);
}

static void op_adc_xh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = XH + arg0 + C;
    x = XL | ((tmp & 0xF) << 4) | (XP << 8);
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_adc_xl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = XL + arg0 + C;
    x = (tmp & 0xF) | (XH << 4) | (XP << 8);
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_adc_yh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = YH + arg0 + C;
    y = YL | ((tmp & 0xF) << 4) | (YP << 8);
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_adc_yl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = YL + arg0 + C;
    y = (tmp & 0xF) | (YH << 4) | (YP << 8);
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!(tmp & 0xF)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_cp_xh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(XH < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(XH == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_cp_xl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(XL < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(XL == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_cp_yh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(YH < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(YH == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_cp_yl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    if(YL < arg0) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(YL == arg0) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_ld_r_i_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, arg1);
}

static void op_ld_r_q_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg1));
}

static void op_ld_a_mn_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    a = M(arg0);
}

static void op_ld_b_mn_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    b = M(arg0);
}

static void op_ld_mn_a_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_M(arg0, a);
}

static void op_ld_mn_b_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_M(arg0, b);
}

static void op_ldpx_mx_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_M(x, arg0);
    x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_ldpx_r_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg1));
    x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_ldpy_my_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_M(y, arg0);
    y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_ldpy_r_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg1));
    y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_lbpx_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_M(x, arg0 & 0xF);
    SET_M(x + 1, (arg0 >> 4) & 0xF);
    x = ((x + 2) & 0xFF) | (XP << 8);
}

static void op_set_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    flags |= arg0;
}

static void op_rst_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    flags &= arg0;
}

static void op_scf_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    SET_C();
}

static void op_rcf_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    CLEAR_C();
}

static void op_szf_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    SET_Z();
}

static void op_rzf_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    CLEAR_Z();
}

static void op_sdf_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    SET_D();
}

static void op_rdf_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    CLEAR_D();
}

static void op_ei_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    SET_I();
}

static void op_di_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    CLEAR_I();
}

static void op_inc_sp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp + 1) & 0xFF;
}

static void op_dec_sp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
}

static void op_push_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, RQ(arg0));
}

static void op_push_xp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, XP);
}

static void op_push_xh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, XH);
}

static void op_push_xl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, XL);
}

static void op_push_yp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, YP);
}

static void op_push_yh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, YH);
}

static void op_push_yl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, YL);
}

static void op_push_f_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    sp = (sp - 1) & 0xFF;
    SET_M(sp, flags);
}

static void op_pop_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, M(sp));
    sp = (sp + 1) & 0xFF;
}

static void op_pop_xp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    x = XL | (XH << 4) | (M(sp) << 8);
    sp = (sp + 1) & 0xFF;
}

static void op_pop_xh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    x = XL | (M(sp) << 4) | (XP << 8);
    sp = (sp + 1) & 0xFF;
}

static void op_pop_xl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    x = M(sp) | (XH << 4) | (XP << 8);
    sp = (sp + 1) & 0xFF;
}

static void op_pop_yp_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    y = YL | (YH << 4) | (M(sp) << 8);
    sp = (sp + 1) & 0xFF;
}

static void op_pop_yh_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    y = YL | (M(sp) << 4) | (YP << 8);
    sp = (sp + 1) & 0xFF;
}

static void op_pop_yl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    y = M(sp) | (YH << 4) | (YP << 8);
    sp = (sp + 1) & 0xFF;
}

static void op_pop_f_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg0);
    UNUSED(arg1);
    flags = M(sp);
    sp = (sp + 1) & 0xFF;
}

static void op_ld_sph_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    sp = SPL | (RQ(arg0) << 4);
}

static void op_ld_spl_r_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    sp = RQ(arg0) | (SPH << 4);
}

static void op_ld_r_sph_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, SPH);
}

static void op_ld_r_spl_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, SPL);
}

static void op_add_r_i_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) + arg1;
    if(D) {
        if(tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if(tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_add_r_q_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) + RQ(arg1);
    if(D) {
        if(tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if(tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_adc_r_i_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) + arg1 + C;
    if(D) {
        if(tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if(tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_adc_r_q_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) + RQ(arg1) + C;
    if(D) {
        if(tmp >= 10) {
            SET_RQ(arg0, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_RQ(arg0, tmp);
            CLEAR_C();
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
        if(tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_sub_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) - RQ(arg1);
    if(D) {
        if(tmp >> 4) {
            SET_RQ(arg0, (tmp - 6) & 0xF);
        } else {
            SET_RQ(arg0, tmp);
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
    }
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_sbc_r_i_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) - arg1 - C;
    if(D) {
        if(tmp >> 4) {
            SET_RQ(arg0, (tmp - 6) & 0xF);
        } else {
            SET_RQ(arg0, tmp);
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
    }
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_sbc_r_q_cb(u8_t arg0, u8_t arg1) {
    u8_t tmp;

    tmp = RQ(arg0) - RQ(arg1) - C;
    if(D) {
        if(tmp >> 4) {
            SET_RQ(arg0, (tmp - 6) & 0xF);
        } else {
            SET_RQ(arg0, tmp);
        }
    } else {
        SET_RQ(arg0, tmp & 0xF);
    }
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_and_r_i_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg0) & arg1);
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_and_r_q_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg0) & RQ(arg1));
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_or_r_i_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg0) | arg1);
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_or_r_q_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg0) | RQ(arg1));
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_xor_r_i_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg0) ^ arg1);
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_xor_r_q_cb(u8_t arg0, u8_t arg1) {
    SET_RQ(arg0, RQ(arg0) ^ RQ(arg1));
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_cp_r_i_cb(u8_t arg0, u8_t arg1) {
    if(RQ(arg0) < arg1) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(RQ(arg0) == arg1) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_cp_r_q_cb(u8_t arg0, u8_t arg1) {
    if(RQ(arg0) < RQ(arg1)) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(RQ(arg0) == RQ(arg1)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_fan_r_i_cb(u8_t arg0, u8_t arg1) {
    if(!(RQ(arg0) & arg1)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_fan_r_q_cb(u8_t arg0, u8_t arg1) {
    if(!(RQ(arg0) & RQ(arg1))) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_rlc_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = (RQ(arg0) << 1) | C;
    if(RQ(arg0) & 0x8) {
        SET_C();
    } else {
        CLEAR_C();
    }
    SET_RQ(arg0, tmp & 0xF);
    /* No need to set Z (issue in DS) */
}

static void op_rrc_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = (RQ(arg0) >> 1) | (C << 3);
    if(RQ(arg0) & 0x1) {
        SET_C();
    } else {
        CLEAR_C();
    }
    SET_RQ(arg0, tmp & 0xF);
    /* No need to set Z (issue in DS) */
}

static void op_inc_mn_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = M(arg0) + 1;
    SET_M(arg0, tmp & 0xF);
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!M(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_dec_mn_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = M(arg0) - 1;
    SET_M(arg0, tmp & 0xF);
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!M(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

static void op_acpx_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = M(x) + RQ(arg0) + C;
    if(D) {
        if(tmp >= 10) {
            SET_M(x, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_M(x, tmp);
            CLEAR_C();
        }
    } else {
        SET_M(x, tmp & 0xF);
        if(tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if(!M(x)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_acpy_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = M(y) + RQ(arg0) + C;
    if(D) {
        if(tmp >= 10) {
            SET_M(y, (tmp - 10) & 0xF);
            SET_C();
        } else {
            SET_M(y, tmp);
            CLEAR_C();
        }
    } else {
        SET_M(y, tmp & 0xF);
        if(tmp >> 4) {
            SET_C();
        } else {
            CLEAR_C();
        }
    }
    if(!M(y)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_scpx_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = M(x) - RQ(arg0) - C;
    if(D) {
        if(tmp >> 4) {
            SET_M(x, (tmp - 6) & 0xF);
        } else {
            SET_M(x, tmp);
        }
    } else {
        SET_M(x, tmp & 0xF);
    }
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!M(x)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_scpy_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    u8_t tmp;

    tmp = M(y) - RQ(arg0) - C;
    if(D) {
        if(tmp >> 4) {
            SET_M(y, (tmp - 6) & 0xF);
        } else {
            SET_M(y, tmp);
        }
    } else {
        SET_M(y, tmp & 0xF);
    }
    if(tmp >> 4) {
        SET_C();
    } else {
        CLEAR_C();
    }
    if(!M(y)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
    y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_not_cb(u8_t arg0, u8_t arg1) {
    UNUSED(arg1);
    SET_RQ(arg0, ~RQ(arg0) & 0xF);
    if(!RQ(arg0)) {
        SET_Z();
    } else {
        CLEAR_Z();
    }
}

/* The E0C6S46 supported instructions */
static const op_t ops[] = {
    {"PSET #0x%02X            ", 0xE40, MASK_7B, 0, 0, 5, &op_pset_cb}, // PSET
    {"JP   #0x%02X            ", 0x000, MASK_4B, 0, 0, 5, &op_jp_cb}, // JP
    {"JP   C #0x%02X          ", 0x200, MASK_4B, 0, 0, 5, &op_jp_c_cb}, // JP_C
    {"JP   NC #0x%02X         ", 0x300, MASK_4B, 0, 0, 5, &op_jp_nc_cb}, // JP_NC
    {"JP   Z #0x%02X          ", 0x600, MASK_4B, 0, 0, 5, &op_jp_z_cb}, // JP_Z
    {"JP   NZ #0x%02X         ", 0x700, MASK_4B, 0, 0, 5, &op_jp_nz_cb}, // JP_NZ
    {"JPBA                  ", 0xFE8, MASK_12B, 0, 0, 5, &op_jpba_cb}, // JPBA
    {"CALL #0x%02X            ", 0x400, MASK_4B, 0, 0, 7, &op_call_cb}, // CALL
    {"CALZ #0x%02X            ", 0x500, MASK_4B, 0, 0, 7, &op_calz_cb}, // CALZ
    {"RET                   ", 0xFDF, MASK_12B, 0, 0, 7, &op_ret_cb}, // RET
    {"RETS                  ", 0xFDE, MASK_12B, 0, 0, 12, &op_rets_cb}, // RETS
    {"RETD #0x%02X            ", 0x100, MASK_4B, 0, 0, 12, &op_retd_cb}, // RETD
    {"NOP5                  ", 0xFFB, MASK_12B, 0, 0, 5, &op_nop5_cb}, // NOP5
    {"NOP7                  ", 0xFFF, MASK_12B, 0, 0, 7, &op_nop7_cb}, // NOP7
    {"HALT                  ", 0xFF8, MASK_12B, 0, 0, 5, &op_halt_cb}, // HALT
    {"INC  X #0x%02X          ", 0xEE0, MASK_12B, 0, 0, 5, &op_inc_x_cb}, // INC_X
    {"INC  Y #0x%02X          ", 0xEF0, MASK_12B, 0, 0, 5, &op_inc_y_cb}, // INC_Y
    {"LD   X #0x%02X          ", 0xB00, MASK_4B, 0, 0, 5, &op_ld_x_cb}, // LD_X
    {"LD   Y #0x%02X          ", 0x800, MASK_4B, 0, 0, 5, &op_ld_y_cb}, // LD_Y
    {"LD   XP R(#0x%02X)      ", 0xE80, MASK_10B, 0, 0, 5, &op_ld_xp_r_cb}, // LD_XP_R
    {"LD   XH R(#0x%02X)      ", 0xE84, MASK_10B, 0, 0, 5, &op_ld_xh_r_cb}, // LD_XH_R
    {"LD   XL R(#0x%02X)      ", 0xE88, MASK_10B, 0, 0, 5, &op_ld_xl_r_cb}, // LD_XL_R
    {"LD   YP R(#0x%02X)      ", 0xE90, MASK_10B, 0, 0, 5, &op_ld_yp_r_cb}, // LD_YP_R
    {"LD   YH R(#0x%02X)      ", 0xE94, MASK_10B, 0, 0, 5, &op_ld_yh_r_cb}, // LD_YH_R
    {"LD   YL R(#0x%02X)      ", 0xE98, MASK_10B, 0, 0, 5, &op_ld_yl_r_cb}, // LD_YL_R
    {"LD   R(#0x%02X) XP      ", 0xEA0, MASK_10B, 0, 0, 5, &op_ld_r_xp_cb}, // LD_R_XP
    {"LD   R(#0x%02X) XH      ", 0xEA4, MASK_10B, 0, 0, 5, &op_ld_r_xh_cb}, // LD_R_XH
    {"LD   R(#0x%02X) XL      ", 0xEA8, MASK_10B, 0, 0, 5, &op_ld_r_xl_cb}, // LD_R_XL
    {"LD   R(#0x%02X) YP      ", 0xEB0, MASK_10B, 0, 0, 5, &op_ld_r_yp_cb}, // LD_R_YP
    {"LD   R(#0x%02X) YH      ", 0xEB4, MASK_10B, 0, 0, 5, &op_ld_r_yh_cb}, // LD_R_YH
    {"LD   R(#0x%02X) YL      ", 0xEB8, MASK_10B, 0, 0, 5, &op_ld_r_yl_cb}, // LD_R_YL
    {"ADC  XH #0x%02X         ", 0xA00, MASK_8B, 0, 0, 7, &op_adc_xh_cb}, // ADC_XH
    {"ADC  XL #0x%02X         ", 0xA10, MASK_8B, 0, 0, 7, &op_adc_xl_cb}, // ADC_XL
    {"ADC  YH #0x%02X         ", 0xA20, MASK_8B, 0, 0, 7, &op_adc_yh_cb}, // ADC_YH
    {"ADC  YL #0x%02X         ", 0xA30, MASK_8B, 0, 0, 7, &op_adc_yl_cb}, // ADC_YL
    {"CP   XH #0x%02X         ", 0xA40, MASK_8B, 0, 0, 7, &op_cp_xh_cb}, // CP_XH
    {"CP   XL #0x%02X         ", 0xA50, MASK_8B, 0, 0, 7, &op_cp_xl_cb}, // CP_XL
    {"CP   YH #0x%02X         ", 0xA60, MASK_8B, 0, 0, 7, &op_cp_yh_cb}, // CP_YH
    {"CP   YL #0x%02X         ", 0xA70, MASK_8B, 0, 0, 7, &op_cp_yl_cb}, // CP_YL
    {"LD   R(#0x%02X) #0x%02X   ", 0xE00, MASK_6B, 4, 0x030, 5, &op_ld_r_i_cb}, // LD_R_I
    {"LD   R(#0x%02X) Q(#0x%02X)", 0xEC0, MASK_8B, 2, 0x00C, 5, &op_ld_r_q_cb}, // LD_R_Q
    {"LD   A M(#0x%02X)       ", 0xFA0, MASK_8B, 0, 0, 5, &op_ld_a_mn_cb}, // LD_A_MN
    {"LD   B M(#0x%02X)       ", 0xFB0, MASK_8B, 0, 0, 5, &op_ld_b_mn_cb}, // LD_B_MN
    {"LD   M(#0x%02X) A       ", 0xF80, MASK_8B, 0, 0, 5, &op_ld_mn_a_cb}, // LD_MN_A
    {"LD   M(#0x%02X) B       ", 0xF90, MASK_8B, 0, 0, 5, &op_ld_mn_b_cb}, // LD_MN_B
    {"LDPX MX #0x%02X         ", 0xE60, MASK_8B, 0, 0, 5, &op_ldpx_mx_cb}, // LDPX_MX
    {"LDPX R(#0x%02X) Q(#0x%02X)", 0xEE0, MASK_8B, 2, 0x00C, 5, &op_ldpx_r_cb}, // LDPX_R
    {"LDPY MY #0x%02X         ", 0xE70, MASK_8B, 0, 0, 5, &op_ldpy_my_cb}, // LDPY_MY
    {"LDPY R(#0x%02X) Q(#0x%02X)", 0xEF0, MASK_8B, 2, 0x00C, 5, &op_ldpy_r_cb}, // LDPY_R
    {"LBPX #0x%02X            ", 0x900, MASK_4B, 0, 0, 5, &op_lbpx_cb}, // LBPX
    {"SET  #0x%02X            ", 0xF40, MASK_8B, 0, 0, 7, &op_set_cb}, // SET
    {"RST  #0x%02X            ", 0xF50, MASK_8B, 0, 0, 7, &op_rst_cb}, // RST
    {"SCF                   ", 0xF41, MASK_12B, 0, 0, 7, &op_scf_cb}, // SCF
    {"RCF                   ", 0xF5E, MASK_12B, 0, 0, 7, &op_rcf_cb}, // RCF
    {"SZF                   ", 0xF42, MASK_12B, 0, 0, 7, &op_szf_cb}, // SZF
    {"RZF                   ", 0xF5D, MASK_12B, 0, 0, 7, &op_rzf_cb}, // RZF
    {"SDF                   ", 0xF44, MASK_12B, 0, 0, 7, &op_sdf_cb}, // SDF
    {"RDF                   ", 0xF5B, MASK_12B, 0, 0, 7, &op_rdf_cb}, // RDF
    {"EI                    ", 0xF48, MASK_12B, 0, 0, 7, &op_ei_cb}, // EI
    {"DI                    ", 0xF57, MASK_12B, 0, 0, 7, &op_di_cb}, // DI
    {"INC  SP               ", 0xFDB, MASK_12B, 0, 0, 5, &op_inc_sp_cb}, // INC_SP
    {"DEC  SP               ", 0xFCB, MASK_12B, 0, 0, 5, &op_dec_sp_cb}, // DEC_SP
    {"PUSH R(#0x%02X)         ", 0xFC0, MASK_10B, 0, 0, 5, &op_push_r_cb}, // PUSH_R
    {"PUSH XP               ", 0xFC4, MASK_12B, 0, 0, 5, &op_push_xp_cb}, // PUSH_XP
    {"PUSH XH               ", 0xFC5, MASK_12B, 0, 0, 5, &op_push_xh_cb}, // PUSH_XH
    {"PUSH XL               ", 0xFC6, MASK_12B, 0, 0, 5, &op_push_xl_cb}, // PUSH_XL
    {"PUSH YP               ", 0xFC7, MASK_12B, 0, 0, 5, &op_push_yp_cb}, // PUSH_YP
    {"PUSH YH               ", 0xFC8, MASK_12B, 0, 0, 5, &op_push_yh_cb}, // PUSH_YH
    {"PUSH YL               ", 0xFC9, MASK_12B, 0, 0, 5, &op_push_yl_cb}, // PUSH_YL
    {"PUSH F                ", 0xFCA, MASK_12B, 0, 0, 5, &op_push_f_cb}, // PUSH_F
    {"POP  R(#0x%02X)         ", 0xFD0, MASK_10B, 0, 0, 5, &op_pop_r_cb}, // POP_R
    {"POP  XP               ", 0xFD4, MASK_12B, 0, 0, 5, &op_pop_xp_cb}, // POP_XP
    {"POP  XH               ", 0xFD5, MASK_12B, 0, 0, 5, &op_pop_xh_cb}, // POP_XH
    {"POP  XL               ", 0xFD6, MASK_12B, 0, 0, 5, &op_pop_xl_cb}, // POP_XL
    {"POP  YP               ", 0xFD7, MASK_12B, 0, 0, 5, &op_pop_yp_cb}, // POP_YP
    {"POP  YH               ", 0xFD8, MASK_12B, 0, 0, 5, &op_pop_yh_cb}, // POP_YH
    {"POP  YL               ", 0xFD9, MASK_12B, 0, 0, 5, &op_pop_yl_cb}, // POP_YL
    {"POP  F                ", 0xFDA, MASK_12B, 0, 0, 5, &op_pop_f_cb}, // POP_F
    {"LD   SPH R(#0x%02X)     ", 0xFE0, MASK_10B, 0, 0, 5, &op_ld_sph_r_cb}, // LD_SPH_R
    {"LD   SPL R(#0x%02X)     ", 0xFF0, MASK_10B, 0, 0, 5, &op_ld_spl_r_cb}, // LD_SPL_R
    {"LD   R(#0x%02X) SPH     ", 0xFE4, MASK_10B, 0, 0, 5, &op_ld_r_sph_cb}, // LD_R_SPH
    {"LD   R(#0x%02X) SPL     ", 0xFF4, MASK_10B, 0, 0, 5, &op_ld_r_spl_cb}, // LD_R_SPL
    {"ADD  R(#0x%02X) #0x%02X   ", 0xC00, MASK_6B, 4, 0x030, 7, &op_add_r_i_cb}, // ADD_R_I
    {"ADD  R(#0x%02X) Q(#0x%02X)", 0xA80, MASK_8B, 2, 0x00C, 7, &op_add_r_q_cb}, // ADD_R_Q
    {"ADC  R(#0x%02X) #0x%02X   ", 0xC40, MASK_6B, 4, 0x030, 7, &op_adc_r_i_cb}, // ADC_R_I
    {"ADC  R(#0x%02X) Q(#0x%02X)", 0xA90, MASK_8B, 2, 0x00C, 7, &op_adc_r_q_cb}, // ADC_R_Q
    {"SUB  R(#0x%02X) Q(#0x%02X)", 0xAA0, MASK_8B, 2, 0x00C, 7, &op_sub_cb}, // SUB
    {"SBC  R(#0x%02X) #0x%02X   ", 0xB40, MASK_6B, 4, 0x030, 7, &op_sbc_r_i_cb}, // SBC_R_I
    {"SBC  R(#0x%02X) Q(#0x%02X)", 0xAB0, MASK_8B, 2, 0x00C, 7, &op_sbc_r_q_cb}, // SBC_R_Q
    {"AND  R(#0x%02X) #0x%02X   ", 0xC80, MASK_6B, 4, 0x030, 7, &op_and_r_i_cb}, // AND_R_I
    {"AND  R(#0x%02X) Q(#0x%02X)", 0xAC0, MASK_8B, 2, 0x00C, 7, &op_and_r_q_cb}, // AND_R_Q
    {"OR   R(#0x%02X) #0x%02X   ", 0xCC0, MASK_6B, 4, 0x030, 7, &op_or_r_i_cb}, // OR_R_I
    {"OR   R(#0x%02X) Q(#0x%02X)", 0xAD0, MASK_8B, 2, 0x00C, 7, &op_or_r_q_cb}, // OR_R_Q
    {"XOR  R(#0x%02X) #0x%02X   ", 0xD00, MASK_6B, 4, 0x030, 7, &op_xor_r_i_cb}, // XOR_R_I
    {"XOR  R(#0x%02X) Q(#0x%02X)", 0xAE0, MASK_8B, 2, 0x00C, 7, &op_xor_r_q_cb}, // XOR_R_Q
    {"CP   R(#0x%02X) #0x%02X   ", 0xDC0, MASK_6B, 4, 0x030, 7, &op_cp_r_i_cb}, // CP_R_I
    {"CP   R(#0x%02X) Q(#0x%02X)", 0xF00, MASK_8B, 2, 0x00C, 7, &op_cp_r_q_cb}, // CP_R_Q
    {"FAN  R(#0x%02X) #0x%02X   ", 0xD80, MASK_6B, 4, 0x030, 7, &op_fan_r_i_cb}, // FAN_R_I
    {"FAN  R(#0x%02X) Q(#0x%02X)", 0xF10, MASK_8B, 2, 0x00C, 7, &op_fan_r_q_cb}, // FAN_R_Q
    {"RLC  R(#0x%02X)         ", 0xAF0, MASK_8B, 0, 0, 7, &op_rlc_cb}, // RLC
    {"RRC  R(#0x%02X)         ", 0xE8C, MASK_10B, 0, 0, 5, &op_rrc_cb}, // RRC
    {"INC  M(#0x%02X)         ", 0xF60, MASK_8B, 0, 0, 7, &op_inc_mn_cb}, // INC_MN
    {"DEC  M(#0x%02X)         ", 0xF70, MASK_8B, 0, 0, 7, &op_dec_mn_cb}, // DEC_MN
    {"ACPX R(#0x%02X)         ", 0xF28, MASK_10B, 0, 0, 7, &op_acpx_cb}, // ACPX
    {"ACPY R(#0x%02X)         ", 0xF2C, MASK_10B, 0, 0, 7, &op_acpy_cb}, // ACPY
    {"SCPX R(#0x%02X)         ", 0xF38, MASK_10B, 0, 0, 7, &op_scpx_cb}, // SCPX
    {"SCPY R(#0x%02X)         ", 0xF3C, MASK_10B, 0, 0, 7, &op_scpy_cb}, // SCPY
    {"NOT  R(#0x%02X)         ", 0xD0F, 0xFCF, 4, 0, 7, &op_not_cb}, // NOT

    {NULL, 0, 0, 0, 0, 0, NULL},
};

static timestamp_t wait_for_cycles(timestamp_t since, u8_t cycles) {
    timestamp_t deadline;

    tick_counter += cycles;

    if(speed_ratio == 0) {
        /* Emulation will be as fast as possible */
        return g_hal->get_timestamp();
    }

    deadline = since + (cycles * ts_freq) / (TICK_FREQUENCY * speed_ratio);
    g_hal->sleep_until(deadline);

    return deadline;
}

static void process_interrupts(void) {
    u8_t i;

    /* Process interrupts in priority order */
    for(i = 0; i < INT_SLOT_NUM; i++) {
        if(interrupts[i].triggered) {
            //printf("IT %u !\n", i);
            SET_M(sp - 1, PCP);
            SET_M(sp - 2, PCSH);
            SET_M(sp - 3, PCSL);
            sp = (sp - 3) & 0xFF;
            CLEAR_I();
            np = TO_NP(NBP, 1);
            pc = TO_PC(PCB, 1, interrupts[i].vector);
            call_depth++;

            ref_ts = wait_for_cycles(ref_ts, 12);
            interrupts[i].triggered = 0;
        }
    }
}

static void print_state(u8_t op_num, u12_t op, u13_t addr) {
    u8_t i;

    if(!g_hal->is_log_enabled(LOG_CPU)) {
        return;
    }

    g_hal->log(LOG_CPU, "0x%04X: ", addr);

    for(i = 0; i < call_depth; i++) {
        g_hal->log(LOG_CPU, "  ");
    }

    if(ops[op_num].mask_arg0 != 0) {
        /* Two arguments */
        g_hal->log(
            LOG_CPU,
            ops[op_num].log,
            (op & ops[op_num].mask_arg0) >> ops[op_num].shift_arg0,
            op & ~(ops[op_num].mask | ops[op_num].mask_arg0));
    } else {
        /* One argument */
        g_hal->log(LOG_CPU, ops[op_num].log, (op & ~ops[op_num].mask) >> ops[op_num].shift_arg0);
    }

    if(call_depth < 10) {
        for(i = 0; i < (10 - call_depth); i++) {
            g_hal->log(LOG_CPU, "  ");
        }
    }

    g_hal->log(LOG_CPU, " ; 0x%03X - ", op);
    for(i = 0; i < 12; i++) {
        g_hal->log(LOG_CPU, "%s", ((op >> (11 - i)) & 0x1) ? "1" : "0");
    }
    g_hal->log(
        LOG_CPU,
        " - PC = 0x%04X, SP = 0x%02X, NP = 0x%02X, X = 0x%03X, Y = 0x%03X, A = 0x%X, B = 0x%X, F = 0x%X\n",
        pc,
        sp,
        np,
        x,
        y,
        a,
        b,
        flags);
}

void cpu_reset(void) {
    u13_t i;

    /* Registers and variables init */
    pc = TO_PC(0, 1, 0x00); // PC starts at bank 0, page 1, step 0
    np = TO_NP(0, 1); // NP starts at page 1
    a = 0; // undef
    b = 0; // undef
    x = 0; // undef
    y = 0; // undef
    sp = 0; // undef
    flags = 0;

    /* Init RAM to zeros */
    for(i = 0; i < MEM_BUFFER_SIZE; i++) {
        memory[i] = 0;
    }

    SET_IO_MEMORY(memory, REG_K40_K43_BZ_OUTPUT_PORT, 0xF); // Output port (R40-R43)
    SET_IO_MEMORY(memory, REG_LCD_CTRL, 0x8); // LCD control
    /* TODO: Input relation register */

    cpu_sync_ref_timestamp();
}

bool_t cpu_init(const u12_t* program, breakpoint_t* breakpoints, u32_t freq) {
    g_program = program;
    g_breakpoints = breakpoints;
    ts_freq = freq;

    cpu_reset();

    return 0;
}

void cpu_release(void) {
}

int cpu_step(void) {
    u12_t op;
    u8_t i;
    breakpoint_t* bp = g_breakpoints;
    static u8_t previous_cycles = 0;

    op = g_program[pc];

    /* Lookup the OP code */
    for(i = 0; ops[i].log != NULL; i++) {
        if((op & ops[i].mask) == ops[i].code) {
            break;
        }
    }

    if(ops[i].log == NULL) {
        g_hal->log(LOG_ERROR, "Unknown op-code 0x%X (pc = 0x%04X)\n", op, pc);
        return 1;
    }

    next_pc = (pc + 1) & 0x1FFF;

    /* Display the operation along with the current state of the processor */
    print_state(i, op, pc);

    /* Match the speed of the real processor
	 * NOTE: For better accuracy, the final wait should happen here, however
	 * the downside is that all interrupts will likely be delayed by one OP
	 */
    ref_ts = wait_for_cycles(ref_ts, previous_cycles);

    /* Process the OP code */
    if(ops[i].cb != NULL) {
        if(ops[i].mask_arg0 != 0) {
            /* Two arguments */
            ops[i].cb(
                (op & ops[i].mask_arg0) >> ops[i].shift_arg0,
                op & ~(ops[i].mask | ops[i].mask_arg0));
        } else {
            /* One arguments */
            ops[i].cb((op & ~ops[i].mask) >> ops[i].shift_arg0, 0);
        }
    }

    /* Prepare for the next instruction */
    pc = next_pc;
    previous_cycles = ops[i].cycles;

    if(i > 0) {
        /* OP code is not PSET, reset NP */
        np = (pc >> 8) & 0x1F;
    }

    /* Handle timers using the internal tick counter */
    if(tick_counter - clk_timer_timestamp >= TIMER_1HZ_PERIOD) {
        do {
            clk_timer_timestamp += TIMER_1HZ_PERIOD;
        } while(tick_counter - clk_timer_timestamp >= TIMER_1HZ_PERIOD);

        generate_interrupt(INT_CLOCK_TIMER_SLOT, 3);
    }

    if(prog_timer_enabled && tick_counter - prog_timer_timestamp >= TIMER_256HZ_PERIOD) {
        do {
            prog_timer_timestamp += TIMER_256HZ_PERIOD;
            prog_timer_data--;

            if(prog_timer_data == 0) {
                prog_timer_data = prog_timer_rld;
                generate_interrupt(INT_PROG_TIMER_SLOT, 0);
            }
        } while(tick_counter - prog_timer_timestamp >= TIMER_256HZ_PERIOD);
    }

    /* Check if there is any pending interrupt */
    if(I && i > 0) { // Do not process interrupts after a PSET operation
        process_interrupts();
    }

    /* Check if we could pause the execution */
    while(bp != NULL) {
        if(bp->addr == pc) {
            return 1;
        }

        bp = bp->next;
    }

    return 0;
}
