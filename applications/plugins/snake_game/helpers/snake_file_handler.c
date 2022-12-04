#include "snake_file_handler.h"

#include <furi.h>
#include <flipper_format/flipper_format.h>

static void snake_game_close_file(FlipperFormat* file) {
    if(file == NULL) {
        furi_record_close(RECORD_STORAGE);
        return;
    }
    flipper_format_file_close(file);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
}

static FlipperFormat* snake_game_open_file() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    if(storage_common_stat(storage, SNAKE_GAME_FILE_PATH, NULL) == FSE_OK) {
        if(!flipper_format_file_open_existing(file, SNAKE_GAME_FILE_PATH)) {
            snake_game_close_file(file);
            return NULL;
        }
    } else {
        if(storage_common_stat(storage, APPS_DATA, NULL) == FSE_NOT_EXIST) {
            if(!storage_simply_mkdir(storage, APPS_DATA)) {
                return NULL;
            }
        }
        if(storage_common_stat(storage, SNAKE_GAME_FILE_DIR_PATH, NULL) == FSE_NOT_EXIST) {
            if(!storage_simply_mkdir(storage, SNAKE_GAME_FILE_DIR_PATH)) {
                return NULL;
            }
        }

        if(!flipper_format_file_open_new(file, SNAKE_GAME_FILE_PATH)) {
            snake_game_close_file(file);
            return NULL;
        }

        flipper_format_write_header_cstr(
            file, SNAKE_GAME_FILE_HEADER, SNAKE_GAME_FILE_ACTUAL_VERSION);
        flipper_format_rewind(file);
    }
    return file;
}

void snake_game_save_score_to_file(int16_t highscore) {
    FlipperFormat* file = snake_game_open_file();
    if(file != NULL) {
        uint32_t temp = highscore;
        if(!flipper_format_insert_or_update_uint32(file, SNAKE_GAME_CONFIG_HIGHSCORE, &temp, 1)) {
            snake_game_close_file(file);
            return;
        }
        snake_game_close_file(file);
    }
}

void snake_game_save_game_to_file(SnakeState* const snake_state) {
    FlipperFormat* file = snake_game_open_file();

    if(file != NULL) {
        uint32_t temp = snake_state->len;
        if(!flipper_format_insert_or_update_uint32(file, SNAKE_GAME_CONFIG_KEY_LEN, &temp, 1)) {
            snake_game_close_file(file);
            return;
        }

        uint16_t array_size = snake_state->len * 2;
        uint32_t temp_array[array_size];
        for(int16_t i = 0, a = 0; a < array_size && i < snake_state->len; i++) {
            temp_array[a++] = snake_state->points[i].x;
            temp_array[a++] = snake_state->points[i].y;
        }
        if(!flipper_format_insert_or_update_uint32(
               file, SNAKE_GAME_CONFIG_KEY_POINTS, temp_array, array_size)) {
            snake_game_close_file(file);
            return;
        }

        temp = snake_state->currentMovement;
        if(!flipper_format_insert_or_update_uint32(
               file, SNAKE_GAME_CONFIG_KEY_CURRENT_MOVEMENT, &temp, 1)) {
            snake_game_close_file(file);
            return;
        }

        temp = snake_state->nextMovement;
        if(!flipper_format_insert_or_update_uint32(
               file, SNAKE_GAME_CONFIG_KEY_NEXT_MOVEMENT, &temp, 1)) {
            snake_game_close_file(file);
            return;
        }

        array_size = 2;
        uint32_t temp_point_array[array_size];
        temp_point_array[0] = snake_state->fruit.x;
        temp_point_array[1] = snake_state->fruit.y;
        if(!flipper_format_insert_or_update_uint32(
               file, SNAKE_GAME_CONFIG_KEY_FRUIT_POINTS, temp_point_array, array_size)) {
            snake_game_close_file(file);
            return;
        }

        snake_game_close_file(file);
    }
}

bool snake_game_init_game_from_file(SnakeState* const snake_state) {
    FlipperFormat* file = snake_game_open_file();

    if(file != NULL) {
        FuriString* file_type = furi_string_alloc();
        uint32_t version = 1;
        if(!flipper_format_read_header(file, file_type, &version)) {
            furi_string_free(file_type);
            snake_game_close_file(file);
            return false;
        }
        furi_string_free(file_type);

        uint32_t temp;
        snake_state->highscore =
            (flipper_format_read_uint32(file, SNAKE_GAME_CONFIG_HIGHSCORE, &temp, 1)) ? temp : 0;
        flipper_format_rewind(file);

        if(!flipper_format_read_uint32(file, SNAKE_GAME_CONFIG_KEY_LEN, &temp, 1)) {
            snake_game_close_file(file);
            return false;
        }
        snake_state->len = temp;
        flipper_format_delete_key(file, SNAKE_GAME_CONFIG_KEY_LEN);

        uint16_t array_size = snake_state->len * 2;
        uint32_t temp_array[array_size];
        if(!flipper_format_read_uint32(
               file, SNAKE_GAME_CONFIG_KEY_POINTS, temp_array, array_size)) {
            snake_game_close_file(file);
            return false;
        }

        for(int16_t i = 0, a = 0; a < array_size && i < snake_state->len; i++) {
            snake_state->points[i].x = temp_array[a++];
            snake_state->points[i].y = temp_array[a++];
        }
        flipper_format_delete_key(file, SNAKE_GAME_CONFIG_KEY_POINTS);

        if(!flipper_format_read_uint32(file, SNAKE_GAME_CONFIG_KEY_CURRENT_MOVEMENT, &temp, 1)) {
            snake_game_close_file(file);
            return false;
        }
        snake_state->currentMovement = temp;
        flipper_format_delete_key(file, SNAKE_GAME_CONFIG_KEY_CURRENT_MOVEMENT);

        if(!flipper_format_read_uint32(file, SNAKE_GAME_CONFIG_KEY_NEXT_MOVEMENT, &temp, 1)) {
            snake_game_close_file(file);
            return false;
        }
        snake_state->nextMovement = temp;
        flipper_format_delete_key(file, SNAKE_GAME_CONFIG_KEY_NEXT_MOVEMENT);

        array_size = 2;
        uint32_t temp_point_array[array_size];
        if(!flipper_format_read_uint32(
               file, SNAKE_GAME_CONFIG_KEY_FRUIT_POINTS, temp_point_array, array_size)) {
            snake_game_close_file(file);
            return false;
        }
        snake_state->fruit.x = temp_point_array[0];
        snake_state->fruit.y = temp_point_array[1];
        flipper_format_delete_key(file, SNAKE_GAME_CONFIG_KEY_FRUIT_POINTS);

        snake_game_close_file(file);

        return true;
    }

    return false;
}
