//
// Created by dwdraugr on 24.11.2021.
//

#include "flipper_chip.h"
#include "flipper_fonts.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

static uint8_t randbyte();
static void draw_sprite(t_chip8_state* state, uint8_t x, uint8_t y, uint8_t n);
static void error_stop(t_chip8_state* state, uint16_t opcode);

t_chip8_state* t_chip8_init(void* (*system_malloc)(size_t)) {
    t_chip8_state* state = system_malloc(sizeof(t_chip8_state));

    state->PC = MEMORY_START_POSITION;
    state->SP = 0;
    state->I = 0;
    state->delay_timer = 0;
    state->sound_timer = 0;
    state->go_render = false;
    state->next_opcode = 0;

    state->memory = system_malloc(MEMORY_SIZE * sizeof(uint8_t));
    //	memset(state->memory, 0, MEMORY_SIZE);
    state->screen = system_malloc(SCREEN_HEIGHT * sizeof(size_t));
    for(int i = 0; i < SCREEN_HEIGHT; i++) {
        state->screen[i] = system_malloc(SCREEN_WIDTH * sizeof(uint8_t));
        //		memset(state->screen[i], 0, SCREEN_WIDTH);
    }
    state->V = system_malloc(CPU_REGISTER_NUMBER * sizeof(uint8_t));
    //	memset(state->V, 0, CPU_REGISTER_NUMBER);
    state->stack = system_malloc(CPU_STACK_DEPTH * sizeof(uint16_t));
    //	memset(state->stack, 0, CPU_STACK_DEPTH * sizeof(short));
    state->key = system_malloc(KEYS_NUMBER * sizeof(uint8_t));
    //	memset(state->key, 0, KEYS_NUMBER);

    memcpy(state->memory, font_small, FONT_SMALL);
    srand(time(NULL));

    return state;
}

bool t_chip8_load_game(t_chip8_state* state, const uint8_t* rom, int rom_size) {
    if(MEMORY_ROM_SIZE < rom_size) {
        return false;
    }
    memcpy(&state->memory[MEMORY_START_POSITION], rom, rom_size);
    return true;
}

void t_chip8_free_memory(t_chip8_state* state, void (*system_free)(void*)) {
    system_free(state->memory);
    for(int i = 0; i < SCREEN_HEIGHT; i++) {
        system_free(state->screen[i]);
    }
    system_free(state->screen);
    system_free(state->V);
    system_free(state->key);
    system_free(state->stack);
    system_free(state);
}

void t_chip8_execute_next_opcode(t_chip8_state* state) {
    static bool isWaitInput = false;
    static uint8_t register_number = 255;

    uint16_t opcode = state->memory[state->PC] << 8 | state->memory[state->PC + 1];
    uint8_t x = (opcode >> 8) & 0x000F;
    uint8_t y = (opcode >> 4) & 0x000F;
    uint8_t n = opcode & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    // jump to input-wait opcode
    if(isWaitInput) {
        opcode = 0xF000;
        kk = 0x0A;
        x = register_number;
    }
    state->current_opcode = opcode & 0xF000;
    switch(opcode & 0xF000) {
    case 0x0000:
        switch(kk) {
        case 0x00E0:
            for(int i = 0; i < SCREEN_HEIGHT; i++) {
                for(int j = 0; j < SCREEN_WIDTH; j++) {
                    state->screen[i][j] = 0;
                }
            }
            state->PC += 2;
            break;
        case 0x00EE:
            state->PC = state->stack[--state->SP];
            break;
        default:
            error_stop(state, opcode);
        }
        break;
    case 0x1000:
        state->PC = nnn;
        break;
    case 0x2000:
        state->stack[state->SP++] = state->PC + 2;
        state->PC = nnn;
        break;
    case 0x3000:
        state->PC += (state->V[x] == kk) ? 4 : 2;
        break;
    case 0x4000:
        state->PC += (state->V[x] != kk) ? 4 : 2;
        break;
    case 0x5000:
        state->PC += (state->V[x] == state->V[y]) ? 4 : 2;
        break;
    case 0x6000:
        state->V[x] = kk;
        state->PC += 2;
        break;
    case 0x7000:
        state->V[x] += kk;
        state->PC += 2;
        break;
    case 0x8000:
        switch(n) {
        case 0x0:
            state->V[x] = state->V[y];
            break;
        case 0x1:
            state->V[x] |= state->V[y];
            break;
        case 0x2:
            state->V[x] &= state->V[y];
            break;
        case 0x3:
            state->V[x] ^= state->V[y];
            break;
        case 0x4:
            state->V[0xF] = (int)state->V[x] + (int)state->V[y] ? 1 : 0;
            state->V[x] += state->V[y];
            break;
        case 0x5:
            state->V[0xF] = state->V[x] > state->V[y] ? 1 : 0;
            state->V[x] -= state->V[y];
            break;
        case 0x6:
            state->V[0xF] = state->V[x] & 0x1;
            state->V[x] >>= 1;
            break;
        case 0x7:
            state->V[0xF] = state->V[y] > state->V[x] ? 1 : 0;
            state->V[x] = state->V[y] - state->V[x];
            break;
        case 0xE:
            state->V[0xF] = (state->V[x] >> 7) & 0x1;
            state->V[x] <<= 1;
            break;
        default:
            error_stop(state, opcode);
        }
        state->PC += 2;
        break;
    case 0x9000:
        switch(n) {
        case 0x0:
            state->PC += state->V[x] != state->V[y] ? 4 : 2;
            break;
        default:
            error_stop(state, opcode);
        }
        break;
    case 0xA000:
        state->I = nnn;
        state->PC += 2;
        break;
    case 0xB000:
        state->PC = nnn + state->V[0];
        break;
    case 0xC000:
        state->V[x] = randbyte() & kk;
        state->PC += 2;
        break;
    case 0xD000:
        draw_sprite(state, state->V[x], state->V[y], n);
        state->go_render = true;
        state->PC += 2;
        break;
    case 0xE000:
        switch(kk) {
        case 0x9E:
            state->PC += state->key[state->V[x]] ? 4 : 2;
            break;
        case 0xA1:
            state->PC += !state->key[state->V[x]] ? 4 : 2;
            break;
        default:
            error_stop(state, opcode);
        }
        break;
    case 0xF000:
        switch(kk) {
        case 0x07:
            state->V[x] = state->delay_timer;
            state->PC += 2;
            break;
        case 0x0A:
            for(int i = 0; i < KEYS_NUMBER; i++) {
                if(state->key[i]) {
                    state->V[x] = i;
                    isWaitInput = false;
                    goto exit_input_wait;
                }
                isWaitInput = true;
                register_number = x;
            }
        exit_input_wait:
            state->PC += 2;
            break;
        case 0x15:
            state->delay_timer = state->V[x];
            state->PC += 2;
            break;
        case 0x18:
            state->sound_timer = state->V[x];
            state->PC += 2;
            break;
        case 0x1E:
            state->V[0xF] = state->I + state->V[x] > 0xFFF ? 1 : 0;
            state->I += state->V[x];
            state->PC += 2;
            break;
        case 0x29:
            state->I = FONT_BYTES_PER_CHAR * state->V[x];
            state->PC += 2;
            break;
        case 0x33:
            state->memory[state->I] = (state->V[x] % 1000) / 100;
            state->memory[state->I + 1] = (state->V[x] % 100) / 10;
            state->memory[state->I + 2] = state->V[x] % 10;
            state->PC += 2;
            break;
        case 0x55:
            memcpy(state->memory, state->V, x);
            state->I += x + 1;
            state->PC += 2;
            break;
        case 0x65:
            for(int i = 0; i <= x; i++) {
                state->V[i] = state->memory[state->I + i];
            }
            state->I += x + 1;
            state->PC += 2;
            break;
        default:
            error_stop(state, opcode);
        }
        break;
    default:
        error_stop(state, opcode);
    }

    state->next_opcode = state->memory[state->PC] << 8 | state->memory[state->PC + 1];
    state->next_opcode &= 0xf000;
}

void t_chip8_tick(t_chip8_state* state) {
    if(state->delay_timer > 0) {
        --state->delay_timer;
    }
    if(state->sound_timer > 0) {
        --state->sound_timer;
    }
}

uint8_t** t_chip8_get_screen(t_chip8_state* state) {
    return (uint8_t**)state->screen;
}

void t_chip8_set_input(t_chip8_state* state, t_keys key) {
    state->key[key] = 1;
}

void t_chip8_release_input(t_chip8_state* state) {
    for(int i = 0; i < KEYS_NUMBER; i++) {
        state->key[i] = 0;
    }
}

static uint8_t randbyte() {
    return rand() % 256;
}

static void draw_sprite(t_chip8_state* state, uint8_t x, uint8_t y, uint8_t n) {
    unsigned row = y, col = x;
    unsigned byte_index;
    unsigned bit_index;

    state->V[0xF] = 0;
    for(byte_index = 0; byte_index < n; byte_index++) {
        uint8_t byte = state->memory[state->I + byte_index];

        for(bit_index = 0; bit_index < 8; bit_index++) {
            uint8_t bit = (byte >> bit_index) & 0x1;

            uint8_t* pixel_pointer = &state->screen[(row + byte_index) % SCREEN_HEIGHT]
                                                   [(col + (7 - bit_index)) % SCREEN_WIDTH];

            if(bit == 1 && *pixel_pointer == 1) state->V[0xF] = 1;
            *pixel_pointer = *pixel_pointer ^ bit;
        }
    }
}

static void error_stop(t_chip8_state* state, uint16_t opcode) {
    exit(100);
}
