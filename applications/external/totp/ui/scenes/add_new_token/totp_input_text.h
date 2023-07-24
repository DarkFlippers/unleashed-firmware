#pragma once

#include <gui/gui.h>

#define INPUT_BUFFER_SIZE (255)

typedef struct {
    char user_input[INPUT_BUFFER_SIZE];
    size_t user_input_length;
    bool success;
} InputTextResult;

void totp_input_text(Gui* gui, const char* header_text, InputTextResult* result);
