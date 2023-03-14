#include "array_utils.h"

void reverse_array(int length, uint8_t arr[length]) {
    uint8_t tmp;
    for(int low = 0, high = length - 1; low < high; low++, high--) {
        tmp = arr[low];
        arr[low] = arr[high];
        arr[high] = tmp;
    }
}

bool shift_array_to_left(int length, uint8_t arr[length], uint8_t from_index, uint8_t offset) {
    if(from_index >= length) return false;
    for(uint8_t i = from_index; i < length; i++) {
        arr[i] = i < length - offset ? arr[i + offset] : 0;
    }
    return true;
}

void get_column_from_array(
    int rows,
    int cols,
    uint8_t arr[rows][cols],
    uint8_t column_index,
    uint8_t* out) {
    for(uint8_t i = 0; i < rows; i++) {
        out[i] = arr[i][column_index];
    }
}

void set_column_to_array(
    int rows,
    int cols,
    uint8_t arr[rows][cols],
    uint8_t column_index,
    uint8_t* src) {
    for(uint8_t i = 0; i < rows; i++) {
        arr[i][column_index] = src[i];
    }
}