#include "barcode_validator.h"

void barcode_loader(BarcodeData* barcode_data) {
    switch(barcode_data->type_obj->type) {
    case UPCA:
    case EAN8:
    case EAN13:
        ean_upc_loader(barcode_data);
        break;
    case CODE39:
        code_39_loader(barcode_data);
        break;
    case CODE128:
        code_128_loader(barcode_data);
        break;
    case CODE128C:
        code_128c_loader(barcode_data);
        break;
    case CODABAR:
        codabar_loader(barcode_data);
        break;
    case UNKNOWN:
        barcode_data->reason = UnsupportedType;
        barcode_data->valid = false;
    default:
        break;
    }
}

/**
 * Calculates the check digit of a barcode if they have one
 * @param barcode_data the barcode data
 * @returns a check digit or -1 for either an invalid 
*/
int calculate_check_digit(BarcodeData* barcode_data) {
    int check_digit = -1;
    switch(barcode_data->type_obj->type) {
    case UPCA:
    case EAN8:
    case EAN13:
        check_digit = calculate_ean_upc_check_digit(barcode_data);
        break;
    case CODE39:
    case CODE128:
    case CODE128C:
    case CODABAR:
    case UNKNOWN:
    default:
        break;
    }

    return check_digit;
}

/**
 * Calculates the check digit of barcode types UPC-A, EAN-8, & EAN-13
*/
int calculate_ean_upc_check_digit(BarcodeData* barcode_data) {
    int check_digit = 0;
    int odd = 0;
    int even = 0;

    int length = barcode_data->type_obj->min_digits;

    //Get sum of odd digits
    for(int i = 0; i < length; i += 2) {
        odd += furi_string_get_char(barcode_data->raw_data, i) - '0';
    }

    //Get sum of even digits
    for(int i = 1; i < length; i += 2) {
        even += furi_string_get_char(barcode_data->raw_data, i) - '0';
    }

    if(barcode_data->type_obj->type == EAN13) {
        check_digit = even * 3 + odd;
    } else {
        check_digit = odd * 3 + even;
    }

    check_digit = check_digit % 10;

    return (10 - check_digit) % 10;
}

/**
 * Loads and validates Barcode Types EAN-8, EAN-13, and UPC-A
 * barcode_data and its strings should already be allocated;
*/
void ean_upc_loader(BarcodeData* barcode_data) {
    int barcode_length = furi_string_size(barcode_data->raw_data);

    int min_digits = barcode_data->type_obj->min_digits;
    int max_digit = barcode_data->type_obj->max_digits;

    //check the length of the barcode
    if(barcode_length < min_digits || barcode_length > max_digit) {
        barcode_data->reason = WrongNumberOfDigits;
        barcode_data->valid = false;
        return;
    }

    //checks if the barcode contains any characters that aren't a number
    for(int i = 0; i < barcode_length; i++) {
        char character = furi_string_get_char(barcode_data->raw_data, i);
        int digit = character - '0'; //convert the number into an int (also the index)
        if(digit < 0 || digit > 9) {
            barcode_data->reason = InvalidCharacters;
            barcode_data->valid = false;
            return;
        }
    }

    int check_digit = calculate_check_digit(barcode_data);
    char check_digit_char = check_digit + '0';

    barcode_data->check_digit = check_digit;

    //if the barcode length is at max length then we will verify if the check digit is correct
    if(barcode_length == max_digit) {
        //append the raw_data to the correct data string
        furi_string_cat(barcode_data->correct_data, barcode_data->raw_data);

        //append the check digit to the correct data string
        furi_string_set_char(barcode_data->correct_data, min_digits, check_digit_char);
    }
    //if the barcode length is at min length, we will calculate the check digit
    if(barcode_length == min_digits) {
        //append the raw_data to the correct data string
        furi_string_cat(barcode_data->correct_data, barcode_data->raw_data);

        //append the check digit to the correct data string
        furi_string_push_back(barcode_data->correct_data, check_digit_char);
    }
}

void code_39_loader(BarcodeData* barcode_data) {
    int barcode_length = furi_string_size(barcode_data->raw_data);

    int min_digits = barcode_data->type_obj->min_digits;

    //check the length of the barcode, must contain atleast a character,
    //this can have as many characters as it wants, it might not fit on the screen
    if(barcode_length < min_digits) {
        barcode_data->reason = WrongNumberOfDigits;
        barcode_data->valid = false;
        return;
    }

    FuriString* barcode_bits = furi_string_alloc();
    FuriString* temp_string = furi_string_alloc();

    //add starting and ending *
    if(!furi_string_start_with(barcode_data->raw_data, "*")) {
        furi_string_push_back(temp_string, '*');
        furi_string_cat(temp_string, barcode_data->raw_data);
        furi_string_set(barcode_data->raw_data, temp_string);
    }

    if(!furi_string_end_with(barcode_data->raw_data, "*")) {
        furi_string_push_back(barcode_data->raw_data, '*');
    }

    furi_string_free(temp_string);
    barcode_length = furi_string_size(barcode_data->raw_data);

    //Open Storage
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    if(!flipper_format_file_open_existing(ff, CODE39_DICT_FILE_PATH)) {
        FURI_LOG_E(TAG, "Could not open file %s", CODE39_DICT_FILE_PATH);
        barcode_data->reason = MissingEncodingTable;
        barcode_data->valid = false;
    } else {
        FuriString* char_bits = furi_string_alloc();
        for(int i = 0; i < barcode_length; i++) {
            char barcode_char = toupper(furi_string_get_char(barcode_data->raw_data, i));

            //convert a char into a string so it used in flipper_format_read_string
            char current_character[2];
            snprintf(current_character, 2, "%c", barcode_char);

            if(!flipper_format_read_string(ff, current_character, char_bits)) {
                FURI_LOG_E(TAG, "Could not read \"%c\" string", barcode_char);
                barcode_data->reason = InvalidCharacters;
                barcode_data->valid = false;
                break;
            } else {
                FURI_LOG_I(
                    TAG, "\"%c\" string: %s", barcode_char, furi_string_get_cstr(char_bits));
                furi_string_cat(barcode_bits, char_bits);
            }
            flipper_format_rewind(ff);
        }
        furi_string_free(char_bits);
    }

    //Close Storage
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    furi_string_cat(barcode_data->correct_data, barcode_bits);
    furi_string_free(barcode_bits);
}

/**
 * Loads a code 128 barcode
 * 
 * Only supports character set B
*/
void code_128_loader(BarcodeData* barcode_data) {
    int barcode_length = furi_string_size(barcode_data->raw_data);

    //the start code for character set B
    int start_code_value = 104;

    //The bits for the start code
    const char* start_code_bits = "11010010000";

    //The bits for the stop code
    const char* stop_code_bits = "1100011101011";

    int min_digits = barcode_data->type_obj->min_digits;

    /**
     * A sum of all of the characters values
     * Ex: 
     * Barcode Data : ABC
     * A has a value of 33
     * B has a value of 34
     * C has a value of 35
     * 
     * the checksum_adder would be (33 * 1) + (34 * 2) + (35 * 3) + 104 = 310
     * 
     * Add 104 since we are using set B
     */
    int checksum_adder = start_code_value;
    /**
     * Checksum digits is the number of characters it has read so far
     * In the above example the checksum_digits would be 3
    */
    int checksum_digits = 0;

    //the calculated check digit
    int final_check_digit = 0;

    //check the length of the barcode, must contain atleast a character,
    //this can have as many characters as it wants, it might not fit on the screen
    if(barcode_length < min_digits) {
        barcode_data->reason = WrongNumberOfDigits;
        barcode_data->valid = false;
        return;
    }

    //Open Storage
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FuriString* barcode_bits = furi_string_alloc();

    //add the start code
    furi_string_cat(barcode_bits, start_code_bits);

    if(!flipper_format_file_open_existing(ff, CODE128_DICT_FILE_PATH)) {
        FURI_LOG_E(TAG, "Could not open file %s", CODE128_DICT_FILE_PATH);
        barcode_data->reason = MissingEncodingTable;
        barcode_data->valid = false;
    } else {
        FuriString* value = furi_string_alloc();
        FuriString* char_bits = furi_string_alloc();
        for(int i = 0; i < barcode_length; i++) {
            char barcode_char = furi_string_get_char(barcode_data->raw_data, i);

            //convert a char into a string so it used in flipper_format_read_string
            char current_character[2];
            snprintf(current_character, 2, "%c", barcode_char);

            //get the value of the character
            if(!flipper_format_read_string(ff, current_character, value)) {
                FURI_LOG_E(TAG, "Could not read \"%c\" string", barcode_char);
                barcode_data->reason = InvalidCharacters;
                barcode_data->valid = false;
                break;
            }
            //using the value of the character, get the characters bits
            if(!flipper_format_read_string(ff, furi_string_get_cstr(value), char_bits)) {
                FURI_LOG_E(TAG, "Could not read \"%c\" string", barcode_char);
                barcode_data->reason = EncodingTableError;
                barcode_data->valid = false;
                break;
            } else {
                //add the bits to the full barcode
                furi_string_cat(barcode_bits, char_bits);

                //calculate the checksum
                checksum_digits += 1;
                checksum_adder += (atoi(furi_string_get_cstr(value)) * checksum_digits);

                FURI_LOG_D(
                    TAG,
                    "\"%c\" string: %s : %s : %d : %d : %d",
                    barcode_char,
                    furi_string_get_cstr(char_bits),
                    furi_string_get_cstr(value),
                    checksum_digits,
                    (atoi(furi_string_get_cstr(value)) * checksum_digits),
                    checksum_adder);
            }
            //bring the file pointer back to the beginning
            flipper_format_rewind(ff);
        }

        //calculate the check digit and convert it into a c string for lookup in the encoding table
        final_check_digit = checksum_adder % 103;
        int length = snprintf(NULL, 0, "%d", final_check_digit);
        char* final_check_digit_string = malloc(length + 1);
        snprintf(final_check_digit_string, length + 1, "%d", final_check_digit);

        //after the checksum has been calculated, add the bits to the full barcode
        if(!flipper_format_read_string(ff, final_check_digit_string, char_bits)) {
            FURI_LOG_E(TAG, "Could not read \"%s\" string", final_check_digit_string);
            barcode_data->reason = EncodingTableError;
            barcode_data->valid = false;
        } else {
            //add the check digit bits to the full barcode
            furi_string_cat(barcode_bits, char_bits);

            FURI_LOG_D(
                TAG,
                "\"%s\" string: %s",
                final_check_digit_string,
                furi_string_get_cstr(char_bits));
        }

        free(final_check_digit_string);
        furi_string_free(value);
        furi_string_free(char_bits);
    }

    //add the stop code
    furi_string_cat(barcode_bits, stop_code_bits);

    //Close Storage
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    furi_string_cat(barcode_data->correct_data, barcode_bits);
    furi_string_free(barcode_bits);
}

/**
 * Loads a code 128 C barcode
*/
void code_128c_loader(BarcodeData* barcode_data) {
    int barcode_length = furi_string_size(barcode_data->raw_data);

    //the start code for character set C
    int start_code_value = 105;

    //The bits for the start code
    const char* start_code_bits = "11010011100";

    //The bits for the stop code
    const char* stop_code_bits = "1100011101011";

    int min_digits = barcode_data->type_obj->min_digits;

    int checksum_adder = start_code_value;
    int checksum_digits = 0;

    //the calculated check digit
    int final_check_digit = 0;

    // check the length of the barcode, must contain atleast 2 character,
    // this can have as many characters as it wants, it might not fit on the screen
    // code 128 C: the length must be even
    if((barcode_length < min_digits) || (barcode_length & 1)) {
        barcode_data->reason = WrongNumberOfDigits;
        barcode_data->valid = false;
        return;
    }
    //Open Storage
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FuriString* barcode_bits = furi_string_alloc();

    //add the start code
    furi_string_cat(barcode_bits, start_code_bits);

    if(!flipper_format_file_open_existing(ff, CODE128C_DICT_FILE_PATH)) {
        FURI_LOG_E(TAG, "c128c Could not open file %s", CODE128C_DICT_FILE_PATH);
        barcode_data->reason = MissingEncodingTable;
        barcode_data->valid = false;
    } else {
        FuriString* value = furi_string_alloc();
        FuriString* char_bits = furi_string_alloc();
        for(int i = 0; i < barcode_length; i += 2) {
            char barcode_char1 = furi_string_get_char(barcode_data->raw_data, i);
            char barcode_char2 = furi_string_get_char(barcode_data->raw_data, i + 1);
            FURI_LOG_I(TAG, "c128c bc1='%c' bc2='%c'", barcode_char1, barcode_char2);

            char current_chars[4];
            snprintf(current_chars, 3, "%c%c", barcode_char1, barcode_char2);
            FURI_LOG_I(TAG, "c128c current_chars='%s'", current_chars);

            //using the value of the characters, get the characters bits
            if(!flipper_format_read_string(ff, current_chars, char_bits)) {
                FURI_LOG_E(TAG, "c128c Could not read \"%s\" string", current_chars);
                barcode_data->reason = EncodingTableError;
                barcode_data->valid = false;
                break;
            } else {
                //add the bits to the full barcode
                furi_string_cat(barcode_bits, char_bits);

                // calculate the checksum
                checksum_digits += 1;
                checksum_adder += (atoi(current_chars) * checksum_digits);

                FURI_LOG_I(
                    TAG,
                    "c128c \"%s\" string: %s : %s : %d : %d : %d",
                    current_chars,
                    furi_string_get_cstr(char_bits),
                    furi_string_get_cstr(value),
                    checksum_digits,
                    (atoi(furi_string_get_cstr(value)) * checksum_digits),
                    checksum_adder);
            }
            //bring the file pointer back to the begining
            flipper_format_rewind(ff);
        }
        //calculate the check digit and convert it into a c string for lookup in the encoding table
        final_check_digit = checksum_adder % 103;
        FURI_LOG_I(TAG, "c128c finale_check_digit=%d", final_check_digit);

        int length = snprintf(NULL, 0, "%d", final_check_digit);
        if(final_check_digit < 100) length = 2;
        char* final_check_digit_string = malloc(length + 1);
        snprintf(final_check_digit_string, length + 1, "%02d", final_check_digit);

        //after the checksum has been calculated, add the bits to the full barcode
        if(!flipper_format_read_string(ff, final_check_digit_string, char_bits)) {
            FURI_LOG_E(TAG, "c128c cksum Could not read \"%s\" string", final_check_digit_string);
            barcode_data->reason = EncodingTableError;
            barcode_data->valid = false;
        } else {
            //add the check digit bits to the full barcode
            furi_string_cat(barcode_bits, char_bits);

            FURI_LOG_I(
                TAG,
                "check digit \"%s\" string: %s",
                final_check_digit_string,
                furi_string_get_cstr(char_bits));
        }

        free(final_check_digit_string);
        furi_string_free(value);
        furi_string_free(char_bits);
    }

    //add the stop code
    furi_string_cat(barcode_bits, stop_code_bits);

    //Close Storage
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    FURI_LOG_I(TAG, "c128c %s", furi_string_get_cstr(barcode_bits));
    furi_string_cat(barcode_data->correct_data, barcode_bits);
    furi_string_free(barcode_bits);
}

void codabar_loader(BarcodeData* barcode_data) {
    int barcode_length = furi_string_size(barcode_data->raw_data);

    int min_digits = barcode_data->type_obj->min_digits;

    //check the length of the barcode, must contain atleast a character,
    //this can have as many characters as it wants, it might not fit on the screen
    if(barcode_length < min_digits) {
        barcode_data->reason = WrongNumberOfDigits;
        barcode_data->valid = false;
        return;
    }

    FuriString* barcode_bits = furi_string_alloc();

    barcode_length = furi_string_size(barcode_data->raw_data);

    //Open Storage
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    if(!flipper_format_file_open_existing(ff, CODABAR_DICT_FILE_PATH)) {
        FURI_LOG_E(TAG, "Could not open file %s", CODABAR_DICT_FILE_PATH);
        barcode_data->reason = MissingEncodingTable;
        barcode_data->valid = false;
    } else {
        FuriString* char_bits = furi_string_alloc();
        for(int i = 0; i < barcode_length; i++) {
            char barcode_char = toupper(furi_string_get_char(barcode_data->raw_data, i));

            //convert a char into a string so it used in flipper_format_read_string
            char current_character[2];
            snprintf(current_character, 2, "%c", barcode_char);

            if(!flipper_format_read_string(ff, current_character, char_bits)) {
                FURI_LOG_E(TAG, "Could not read \"%c\" string", barcode_char);
                barcode_data->reason = InvalidCharacters;
                barcode_data->valid = false;
                break;
            } else {
                FURI_LOG_I(
                    TAG, "\"%c\" string: %s", barcode_char, furi_string_get_cstr(char_bits));
                furi_string_cat(barcode_bits, char_bits);
            }
            flipper_format_rewind(ff);
        }
        furi_string_free(char_bits);
    }

    //Close Storage
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    furi_string_cat(barcode_data->correct_data, barcode_bits);
    furi_string_free(barcode_bits);
}
