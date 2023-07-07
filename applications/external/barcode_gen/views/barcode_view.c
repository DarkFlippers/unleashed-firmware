#include "../barcode_app.h"
#include "barcode_view.h"
#include "../encodings.h"

/**
 * @brief Draws a single bit from a barcode at a specified location
 * @param canvas 
 * @param bit  a 1 or a 0 to signify a bit of data
 * @param x  the top left x coordinate
 * @param y  the top left y coordinate
 * @param width  the width of the bit
 * @param height  the height of the bit
 */
static void draw_bit(Canvas* canvas, int bit, int x, int y, int width, int height) {
    if(bit == 1) {
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_box(canvas, x, y, width, height);
}

/**
 * 
*/
static void draw_error_str(Canvas* canvas, const char* error) {
    canvas_clear(canvas);
    canvas_draw_str_aligned(canvas, 62, 30, AlignCenter, AlignCenter, error);
}

/**
 * @param bits  a string of 1's and 0's
 * @returns the x coordinate after the bits have been drawn, useful for drawing the next section of bits
*/
static int draw_bits(Canvas* canvas, const char* bits, int x, int y, int width, int height) {
    int bits_length = strlen(bits);
    for(int i = 0; i < bits_length; i++) {
        char c = bits[i];
        int num = c - '0';

        draw_bit(canvas, num, x, y, width, height);

        x += width;
    }
    return x;
}

/**
 * Draws an EAN-8 type barcode, does not check if the barcode is valid
 * @param canvas  the canvas
 * @param barcode_digits  the digits in the barcode, must be 8 characters long
*/
static void draw_ean_8(Canvas* canvas, BarcodeData* barcode_data) {
    FuriString* barcode_digits = barcode_data->correct_data;
    BarcodeTypeObj* type_obj = barcode_data->type_obj;

    int barcode_length = furi_string_size(barcode_digits);

    int x = type_obj->start_pos;
    int y = BARCODE_Y_START;
    int width = 1;
    int height = BARCODE_HEIGHT;

    //the guard patterns for the beginning, center, ending
    const char* end_bits = "101";
    const char* center_bits = "01010";

    //draw the starting guard pattern
    x = draw_bits(canvas, end_bits, x, y, width, height + 5);

    FuriString* code_part = furi_string_alloc();

    //loop through each digit, find the encoding, and draw it
    for(int i = 0; i < barcode_length; i++) {
        char current_digit = furi_string_get_char(barcode_digits, i);

        //the actual number and the index of the bits
        int index = current_digit - '0';
        //use the L-codes for the first 4 digits and the R-Codes for the last 4 digits
        if(i <= 3) {
            furi_string_set_str(code_part, UPC_EAN_L_CODES[index]);
        } else {
            furi_string_set_str(code_part, UPC_EAN_R_CODES[index]);
        }

        //convert the current_digit char into a string so it can be printed
        char current_digit_string[2];
        snprintf(current_digit_string, 2, "%c", current_digit);

        //set the canvas color to black to print the digit
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_str(canvas, x + 1, y + height + 8, current_digit_string);

        //draw the bits of the barcode
        x = draw_bits(canvas, furi_string_get_cstr(code_part), x, y, width, height);

        //if the index has reached 3, that means 4 digits have been drawn and now draw the center guard pattern
        if(i == 3) {
            x = draw_bits(canvas, center_bits, x, y, width, height + 5);
        }
    }
    furi_string_free(code_part);

    //draw the ending guard pattern
    x = draw_bits(canvas, end_bits, x, y, width, height + 5);
}

static void draw_ean_13(Canvas* canvas, BarcodeData* barcode_data) {
    FuriString* barcode_digits = barcode_data->correct_data;
    BarcodeTypeObj* type_obj = barcode_data->type_obj;

    int barcode_length = furi_string_size(barcode_digits);

    int x = type_obj->start_pos;
    int y = BARCODE_Y_START;
    int width = 1;
    int height = BARCODE_HEIGHT;

    //the guard patterns for the beginning, center, ending
    const char* end_bits = "101";
    const char* center_bits = "01010";

    //draw the starting guard pattern
    x = draw_bits(canvas, end_bits, x, y, width, height + 5);

    FuriString* left_structure = furi_string_alloc();
    FuriString* code_part = furi_string_alloc();

    //loop through each digit, find the encoding, and draw it
    for(int i = 0; i < barcode_length; i++) {
        char current_digit = furi_string_get_char(barcode_digits, i);
        int index = current_digit - '0';

        if(i == 0) {
            furi_string_set_str(left_structure, EAN_13_STRUCTURE_CODES[index]);

            //convert the current_digit char into a string so it can be printed
            char current_digit_string[2];
            snprintf(current_digit_string, 2, "%c", current_digit);

            //set the canvas color to black to print the digit
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_str(canvas, x - 10, y + height + 8, current_digit_string);

            continue;
        } else {
            //use the L-codes for the first 6 digits and the R-Codes for the last 6 digits
            if(i <= 6) {
                //get the encoding type at the current barcode bit position
                char encoding_type = furi_string_get_char(left_structure, i - 1);
                if(encoding_type == 'L') {
                    furi_string_set_str(code_part, UPC_EAN_L_CODES[index]);
                } else {
                    furi_string_set_str(code_part, EAN_G_CODES[index]);
                }
            } else {
                furi_string_set_str(code_part, UPC_EAN_R_CODES[index]);
            }

            //convert the current_digit char into a string so it can be printed
            char current_digit_string[2];
            snprintf(current_digit_string, 2, "%c", current_digit);

            //set the canvas color to black to print the digit
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_str(canvas, x + 1, y + height + 8, current_digit_string);

            //draw the bits of the barcode
            x = draw_bits(canvas, furi_string_get_cstr(code_part), x, y, width, height);

            //if the index has reached 6, that means 6 digits have been drawn and we now draw the center guard pattern
            if(i == 6) {
                x = draw_bits(canvas, center_bits, x, y, width, height + 5);
            }
        }
    }

    furi_string_free(left_structure);
    furi_string_free(code_part);

    //draw the ending guard pattern
    x = draw_bits(canvas, end_bits, x, y, width, height + 5);
}

/**
 * Draw a UPC-A barcode
*/
static void draw_upc_a(Canvas* canvas, BarcodeData* barcode_data) {
    FuriString* barcode_digits = barcode_data->correct_data;
    BarcodeTypeObj* type_obj = barcode_data->type_obj;

    int barcode_length = furi_string_size(barcode_digits);

    int x = type_obj->start_pos;
    int y = BARCODE_Y_START;
    int width = 1;
    int height = BARCODE_HEIGHT;

    //the guard patterns for the beginning, center, ending
    char* end_bits = "101";
    char* center_bits = "01010";

    //draw the starting guard pattern
    x = draw_bits(canvas, end_bits, x, y, width, height + 5);

    FuriString* code_part = furi_string_alloc();

    //loop through each digit, find the encoding, and draw it
    for(int i = 0; i < barcode_length; i++) {
        char current_digit = furi_string_get_char(barcode_digits, i);
        int index = current_digit - '0'; //convert the number into an int (also the index)

        //use the L-codes for the first 6 digits and the R-Codes for the last 6 digits
        if(i <= 5) {
            furi_string_set_str(code_part, UPC_EAN_L_CODES[index]);
        } else {
            furi_string_set_str(code_part, UPC_EAN_R_CODES[index]);
        }

        //convert the current_digit char into a string so it can be printed
        char current_digit_string[2];
        snprintf(current_digit_string, 2, "%c", current_digit);

        //set the canvas color to black to print the digit
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_str(canvas, x + 1, y + height + 8, current_digit_string);

        //draw the bits of the barcode
        x = draw_bits(canvas, furi_string_get_cstr(code_part), x, y, width, height);

        //if the index has reached 6, that means 6 digits have been drawn and we now draw the center guard pattern
        if(i == 5) {
            x = draw_bits(canvas, center_bits, x, y, width, height + 5);
        }
    }

    furi_string_free(code_part);

    //draw the ending guard pattern
    x = draw_bits(canvas, end_bits, x, y, width, height + 5);
}

static void draw_code_39(Canvas* canvas, BarcodeData* barcode_data) {
    FuriString* raw_data = barcode_data->raw_data;
    FuriString* barcode_digits = barcode_data->correct_data;
    //BarcodeTypeObj* type_obj = barcode_data->type_obj;

    int barcode_length = furi_string_size(barcode_digits);
    int total_pixels = 0;

    for(int i = 0; i < barcode_length; i++) {
        //1 for wide, 0 for narrow
        char wide_or_narrow = furi_string_get_char(barcode_digits, i);
        int wn_digit = wide_or_narrow - '0'; //wide(1) or narrow(0) digit

        if(wn_digit == 1) {
            total_pixels += 3;
        } else {
            total_pixels += 1;
        }
        if((i + 1) % 9 == 0) {
            total_pixels += 1;
        }
    }

    int x = (128 - total_pixels) / 2;
    int y = BARCODE_Y_START;
    int width = 1;
    int height = BARCODE_HEIGHT;
    bool filled_in = true;

    //set the canvas color to black to print the digit
    canvas_set_color(canvas, ColorBlack);
    // canvas_draw_str_aligned(canvas, 62, 30, AlignCenter, AlignCenter, error);
    canvas_draw_str_aligned(
        canvas, 62, y + height + 8, AlignCenter, AlignBottom, furi_string_get_cstr(raw_data));

    for(int i = 0; i < barcode_length; i++) {
        //1 for wide, 0 for narrow
        char wide_or_narrow = furi_string_get_char(barcode_digits, i);
        int wn_digit = wide_or_narrow - '0'; //wide(1) or narrow(0) digit

        if(filled_in) {
            if(wn_digit == 1) {
                x = draw_bits(canvas, "111", x, y, width, height);
            } else {
                x = draw_bits(canvas, "1", x, y, width, height);
            }
            filled_in = false;
        } else {
            if(wn_digit == 1) {
                x = draw_bits(canvas, "000", x, y, width, height);
            } else {
                x = draw_bits(canvas, "0", x, y, width, height);
            }
            filled_in = true;
        }
        if((i + 1) % 9 == 0) {
            x = draw_bits(canvas, "0", x, y, width, height);
            filled_in = true;
        }
    }
}

static void draw_code_128(Canvas* canvas, BarcodeData* barcode_data) {
    FuriString* raw_data = barcode_data->raw_data;
    FuriString* barcode_digits = barcode_data->correct_data;

    int barcode_length = furi_string_size(barcode_digits);

    int x = (128 - barcode_length) / 2;
    int y = BARCODE_Y_START;
    int width = 1;
    int height = BARCODE_HEIGHT;

    x = draw_bits(canvas, furi_string_get_cstr(barcode_digits), x, y, width, height);

    //set the canvas color to black to print the digit
    canvas_set_color(canvas, ColorBlack);
    // canvas_draw_str_aligned(canvas, 62, 30, AlignCenter, AlignCenter, error);
    canvas_draw_str_aligned(
        canvas, 62, y + height + 8, AlignCenter, AlignBottom, furi_string_get_cstr(raw_data));
}

static void draw_codabar(Canvas* canvas, BarcodeData* barcode_data) {
    FuriString* raw_data = barcode_data->raw_data;
    FuriString* barcode_digits = barcode_data->correct_data;
    //BarcodeTypeObj* type_obj = barcode_data->type_obj;

    int barcode_length = furi_string_size(barcode_digits);
    int total_pixels = 0;

    for(int i = 0; i < barcode_length; i++) {
        //1 for wide, 0 for narrow
        char wide_or_narrow = furi_string_get_char(barcode_digits, i);
        int wn_digit = wide_or_narrow - '0'; //wide(1) or narrow(0) digit

        if(wn_digit == 1) {
            total_pixels += 3;
        } else {
            total_pixels += 1;
        }
        if((i + 1) % 7 == 0) {
            total_pixels += 1;
        }
    }

    int x = (128 - total_pixels) / 2;
    int y = BARCODE_Y_START;
    int width = 1;
    int height = BARCODE_HEIGHT;
    bool filled_in = true;

    //set the canvas color to black to print the digit
    canvas_set_color(canvas, ColorBlack);
    // canvas_draw_str_aligned(canvas, 62, 30, AlignCenter, AlignCenter, error);
    canvas_draw_str_aligned(
        canvas, 62, y + height + 8, AlignCenter, AlignBottom, furi_string_get_cstr(raw_data));

    for(int i = 0; i < barcode_length; i++) {
        //1 for wide, 0 for narrow
        char wide_or_narrow = furi_string_get_char(barcode_digits, i);
        int wn_digit = wide_or_narrow - '0'; //wide(1) or narrow(0) digit

        if(filled_in) {
            if(wn_digit == 1) {
                x = draw_bits(canvas, "111", x, y, width, height);
            } else {
                x = draw_bits(canvas, "1", x, y, width, height);
            }
            filled_in = false;
        } else {
            if(wn_digit == 1) {
                x = draw_bits(canvas, "000", x, y, width, height);
            } else {
                x = draw_bits(canvas, "0", x, y, width, height);
            }
            filled_in = true;
        }
        if((i + 1) % 7 == 0) {
            x = draw_bits(canvas, "0", x, y, width, height);
            filled_in = true;
        }
    }
}

static void barcode_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    BarcodeModel* barcode_model = ctx;
    BarcodeData* data = barcode_model->data;
    // const char* barcode_digits =;

    canvas_clear(canvas);
    if(data->valid) {
        switch(data->type_obj->type) {
        case UPCA:
            draw_upc_a(canvas, data);
            break;
        case EAN8:
            draw_ean_8(canvas, data);
            break;
        case EAN13:
            draw_ean_13(canvas, data);
            break;
        case CODE39:
            draw_code_39(canvas, data);
            break;
        case CODE128:
        case CODE128C:
            draw_code_128(canvas, data);
            break;
        case CODABAR:
            draw_codabar(canvas, data);
            break;
        case UNKNOWN:
        default:
            break;
        }
    } else {
        switch(data->reason) {
        case WrongNumberOfDigits:
            draw_error_str(canvas, "Wrong # of characters");
            break;
        case InvalidCharacters:
            draw_error_str(canvas, "Invalid characters");
            break;
        case UnsupportedType:
            draw_error_str(canvas, "Unsupported barcode type");
            break;
        case FileOpening:
            draw_error_str(canvas, "Could not open file");
            break;
        case InvalidFileData:
            draw_error_str(canvas, "Invalid file data");
            break;
        case MissingEncodingTable:
            draw_error_str(canvas, "Missing encoding table");
            break;
        case EncodingTableError:
            draw_error_str(canvas, "Encoding table error");
            break;
        default:
            draw_error_str(canvas, "Could not read barcode data");
            break;
        }
    }
}

bool barcode_input_callback(InputEvent* input_event, void* ctx) {
    UNUSED(ctx);
    //furi_assert(ctx);

    //Barcode* test_view_object = ctx;

    if(input_event->key == InputKeyBack) {
        return false;
    } else {
        return true;
    }
}

Barcode* barcode_view_allocate(BarcodeApp* barcode_app) {
    furi_assert(barcode_app);

    Barcode* barcode = malloc(sizeof(Barcode));

    barcode->view = view_alloc();
    barcode->barcode_app = barcode_app;

    view_set_context(barcode->view, barcode);
    view_allocate_model(barcode->view, ViewModelTypeLocking, sizeof(BarcodeModel));
    view_set_draw_callback(barcode->view, barcode_draw_callback);
    view_set_input_callback(barcode->view, barcode_input_callback);

    return barcode;
}

void barcode_free_model(Barcode* barcode) {
    with_view_model(
        barcode->view,
        BarcodeModel * model,
        {
            if(model->file_path != NULL) {
                furi_string_free(model->file_path);
            }
            if(model->data != NULL) {
                if(model->data->raw_data != NULL) {
                    furi_string_free(model->data->raw_data);
                }
                if(model->data->correct_data != NULL) {
                    furi_string_free(model->data->correct_data);
                }
                free(model->data);
            }
        },
        false);
}

void barcode_free(Barcode* barcode) {
    furi_assert(barcode);

    barcode_free_model(barcode);
    view_free(barcode->view);
    free(barcode);
}

View* barcode_get_view(Barcode* barcode) {
    furi_assert(barcode);
    return barcode->view;
}
