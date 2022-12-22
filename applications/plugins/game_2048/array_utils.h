#pragma once

#include <stdint.h>
#include <stdbool.h>

void reverse_array(int length, uint8_t arr[length]);

bool shift_array_to_left(int length, uint8_t arr[length], uint8_t from_index, uint8_t offset);

void get_column_from_array(
    int rows,
    int cols,
    uint8_t arr[rows][cols],
    uint8_t column_index,
    uint8_t* out);

void set_column_to_array(
    int rows,
    int cols,
    uint8_t arr[rows][cols],
    uint8_t column_index,
    uint8_t* src);
