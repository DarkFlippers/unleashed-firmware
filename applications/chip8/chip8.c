#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <storage/storage.h>
#include "chip8.h"
#include "emulator_core/flipper_chip.h"

#define TAG "Chip8Emulator"
#define WORKER_TAG TAG "Worker"
#define FILE_BUFFER_LEN 16

typedef enum {
    WorkerEvtToggle = (1 << 1),
    WorkerEvtEnd = (1 << 2),
} WorkerEvtFlags;

struct Chip8Emulator {
    Chip8State st;
    string_t file_path;
    FuriThread* thread;
};

static int32_t chip8_worker(void* context) {
    Chip8Emulator* chip8 = context;

    FURI_LOG_I(WORKER_TAG, "Start furi record open");
    Storage* furi_storage_record = furi_record_open("storage");
    FURI_LOG_I(WORKER_TAG, "furi record opened");

    FURI_LOG_I(WORKER_TAG, "Start storage file alloc");
    File* rom_file = storage_file_alloc(furi_storage_record);
    FURI_LOG_I(
        WORKER_TAG, "Start storage file open, path = %s", string_get_cstr(chip8->file_path));

    uint8_t* rom_data = malloc(4096);
    FURI_LOG_I(WORKER_TAG, "4096 array gotten");

    while(1) {
        if(chip8->st.worker_state == WorkerStateBackPressed) {
            FURI_LOG_I(WORKER_TAG, "WorkerStateBackPressed");
            break;
        }

        if(chip8->st.worker_state == WorkerStateLoadingRom) {
            bool is_file_opened = storage_file_open(
                rom_file, string_get_cstr(chip8->file_path), FSAM_READ, FSOM_OPEN_EXISTING);

            if(!is_file_opened) {
                FURI_LOG_I(WORKER_TAG, "Cannot open storage");
                storage_file_close(rom_file);
                storage_file_free(rom_file);
                chip8->st.worker_state = WorkerStateRomLoadError;
                return 0;
            }

            FURI_LOG_I(WORKER_TAG, "File was opened, try read this");

            int rom_len = read_rom_data(rom_file, rom_data);

            FURI_LOG_I(WORKER_TAG, "Rom data finished reading");

            FURI_LOG_I(WORKER_TAG, "Load chip8 core data");
            t_chip8_load_game(chip8->st.t_chip8_state, rom_data, rom_len);
            FURI_LOG_I(WORKER_TAG, "chip8 core data loaded");

            FURI_LOG_I(WORKER_TAG, "Wipe screen start");
            for(int i = 0; i < CHIP8_SCREEN_H; i++) {
                FURI_LOG_I(WORKER_TAG, "Wipe screen line %d", i);
                for(int j = 0; j < CHIP8_SCREEN_W; j++) {
                    chip8->st.t_chip8_state->screen[i][j] = 0;
                }
                furi_delay_ms(15);
            }
            FURI_LOG_I(WORKER_TAG, "Wipe screen end");

            chip8->st.worker_state = WorkerStateRomLoaded;
        }

        if(chip8->st.worker_state == WorkerStateRomLoaded) {
            if(chip8->st.t_chip8_state->go_render) {
                continue;
            }
            t_chip8_execute_next_opcode(chip8->st.t_chip8_state);
            FURI_LOG_I(
                "chip8_executing",
                "current: 0x%X next: 0x%X",
                chip8->st.t_chip8_state->current_opcode,
                chip8->st.t_chip8_state->next_opcode);
            furi_delay_ms(2);
            //t_chip8_tick(chip8->st.t_chip8_state);
        }
    }

    FURI_LOG_I("CHIP8", "Prepare to ending app");
    storage_file_close(rom_file);
    storage_file_free(rom_file);
    t_chip8_free_memory(chip8->st.t_chip8_state, free);
    FURI_LOG_I("CHIP8", "End ending");
    return 0;
}

Chip8Emulator* chip8_make_emulator(string_t file_path) {
    furi_assert(file_path);
    FURI_LOG_I("CHIP8", "make emulator, file_path=", string_get_cstr(file_path));

    Chip8Emulator* chip8 = malloc(sizeof(Chip8Emulator));
    string_init(chip8->file_path);
    string_set(chip8->file_path, file_path);
    chip8->st.worker_state = WorkerStateLoadingRom;
    chip8->st.t_chip8_state = t_chip8_init(malloc);

    //    FURI_LOG_I(WORKER_TAG, "Start wipe screen");
    //    furi_delay_ms(1500);
    //    for (int i = 0; i < CHIP8_SCREEN_H; i++)
    //    {
    //        FURI_LOG_I(WORKER_TAG, "Start wipe line %d", i);
    //        for (int j = 0; j < CHIP8_SCREEN_W; j++)
    //        {
    //            chip8->st.t_chip8_state->screen[i][j] = 0;
    //        }
    //    }
    //    FURI_LOG_I(WORKER_TAG, "End wipe screen");

    chip8->thread = furi_thread_alloc();
    furi_thread_set_name(chip8->thread, "Chip8Worker");
    furi_thread_set_stack_size(chip8->thread, 4096);
    furi_thread_set_context(chip8->thread, chip8);
    furi_thread_set_callback(chip8->thread, chip8_worker);

    furi_thread_start(chip8->thread);
    return chip8;
}

void chip8_close_emulator(Chip8Emulator* chip8) {
    FURI_LOG_I("chip_8_close_emulator", "start");
    furi_assert(chip8);
    furi_thread_flags_set(furi_thread_get_id(chip8->thread), WorkerEvtEnd);
    furi_thread_join(chip8->thread);
    furi_thread_free(chip8->thread);
    string_clear(chip8->file_path);
    free(chip8);
    FURI_LOG_I("chip_8_close_emulator", "end");
}

void chip8_toggle(Chip8Emulator* chip8) {
    furi_assert(chip8);
    furi_thread_flags_set(furi_thread_get_id(chip8->thread), WorkerEvtToggle);
}

Chip8State* chip8_get_state(Chip8Emulator* chip8) {
    furi_assert(chip8);
    return &(chip8->st);
}

uint16_t read_rom_data(File* file, uint8_t* data) {
    furi_assert(file);
    furi_assert(data);

    const uint8_t buffer_size = 32;
    uint16_t file_pointer = 0;
    uint8_t buff[buffer_size];

    while(1) {
        uint16_t bytes_were_read = storage_file_read(file, buff, buffer_size);

        if(bytes_were_read == 0) {
            break;
        }

        for(uint16_t i = 0; i < bytes_were_read; i++) {
            data[file_pointer] = buff[i];
            file_pointer++;
        }
    }

    return file_pointer;
}

void chip8_set_back_pressed(Chip8Emulator* chip8) {
    chip8->st.worker_state = WorkerStateBackPressed;
    chip8->st.t_chip8_state->go_render = true;
    FURI_LOG_I(WORKER_TAG, "SET BACK PRESSED. EMULATION IS STOPPED");
}

void chip8_set_up_pressed(Chip8Emulator* chip8) {
    chip8->st.t_chip8_state->go_render = true;
    t_chip8_set_input(chip8->st.t_chip8_state, k_1);
    FURI_LOG_I(WORKER_TAG, "UP PRESSED");
}

void chip8_set_down_pressed(Chip8Emulator* chip8) {
    chip8->st.t_chip8_state->go_render = true;
    t_chip8_set_input(chip8->st.t_chip8_state, k_4);
    FURI_LOG_I(WORKER_TAG, "DOWN PRESSED");
}

void chip8_release_keyboard(Chip8Emulator* chip8) {
    chip8->st.t_chip8_state->go_render = true;
    t_chip8_release_input(chip8->st.t_chip8_state);
    FURI_LOG_I(WORKER_TAG, "chip8_release_keyboard Release input");
}
