#include "multi_converter_units.h"

#define MULTI_CONVERTER_CHAR_OVERFLOW '#'
#define MULTI_CONVERTER_MAX_SUPORTED_INT 999999999

#define multi_converter_unit_set_overflow(b)                  \
    for(int _i = 0; _i < MULTI_CONVERTER_NUMBER_DIGITS; _i++) \
        b[_i] = MULTI_CONVERTER_CHAR_OVERFLOW;

//
// DEC / HEX / BIN conversion
//
void multi_converter_unit_dec_hex_bin_convert(MultiConverterState* const multi_converter_state) {
    char dest[MULTI_CONVERTER_NUMBER_DIGITS];

    int i = 0;
    uint8_t overflow = 0;

    int a = 0;
    int r = 0;
    uint8_t f = 1;

    switch(multi_converter_state->unit_type_orig) {
    default:
        break;
    case UnitTypeDec: {
        a = atoi(multi_converter_state->buffer_orig);
        f = (multi_converter_state->unit_type_dest == UnitTypeHex ? 16 : 2);

        break;
    }
    case UnitTypeHex:
        a = strtol(multi_converter_state->buffer_orig, NULL, 16);
        f = (multi_converter_state->unit_type_dest == UnitTypeDec ? 10 : 2);

        break;
    case UnitTypeBin:
        a = strtol(multi_converter_state->buffer_orig, NULL, 2);
        f = (multi_converter_state->unit_type_dest == UnitTypeDec ? 10 : 16);

        break;
    }

    while(a > 0) {
        r = a % f;
        dest[i] = r + (r < 10 ? '0' : ('A' - 10));
        a /= f;
        if(i++ >= MULTI_CONVERTER_NUMBER_DIGITS) {
            overflow = 1;
            break;
        }
    }

    if(overflow) {
        multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    } else {
        // copy DEST (reversed) to destination and append empty chars at the end
        for(int j = 0; j < MULTI_CONVERTER_NUMBER_DIGITS; j++) {
            if(i >= 1)
                multi_converter_state->buffer_dest[j] = dest[--i];
            else
                multi_converter_state->buffer_dest[j] = ' ';
        }
    }
}

uint8_t multi_converter_unit_dec_hex_bin_allowed(MultiConverterUnitType unit_type) {
    return (unit_type == UnitTypeDec || unit_type == UnitTypeHex || unit_type == UnitTypeBin);
}

//
// CEL / FAR / KEL
//
void multi_converter_unit_temperature_convert(MultiConverterState* const multi_converter_state) {
    double a = strtof(multi_converter_state->buffer_orig, NULL);
    uint8_t overflow = 0;

    switch(multi_converter_state->unit_type_orig) {
    default:
        break;
    case UnitTypeCelsius:
        if(multi_converter_state->unit_type_dest == UnitTypeFahernheit) {
            // celsius to fahrenheit
            a = (a * ((double)1.8)) + 32;
        } else { // UnitTypeKelvin
            a += ((double)273.15);
        }

        break;
    case UnitTypeFahernheit:
        // fahrenheit to celsius, always
        a = (a - 32) / ((double)1.8);
        if(multi_converter_state->unit_type_dest == UnitTypeKelvin) {
            // if kelvin, add
            a += ((double)273.15);
        }

        break;
    case UnitTypeKelvin:
        // kelvin to celsius, always
        a -= ((double)273.15);
        if(multi_converter_state->unit_type_dest == UnitTypeFahernheit) {
            // if fahernheit, convert
            a = (a * ((double)1.8)) + 32;
        }

        break;
    }

    if(overflow) {
        multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    } else {
        int ret = snprintf(
            multi_converter_state->buffer_dest, MULTI_CONVERTER_NUMBER_DIGITS + 1, "%.3lf", a);

        if(ret < 0) multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    }
}

uint8_t multi_converter_unit_temperature_allowed(MultiConverterUnitType unit_type) {
    return (
        unit_type == UnitTypeCelsius || unit_type == UnitTypeFahernheit ||
        unit_type == UnitTypeKelvin);
}

//
// KM / M / CM / MILES / FEET / INCHES
//

void multi_converter_unit_distance_convert(MultiConverterState* const multi_converter_state) {
    double a = strtof(multi_converter_state->buffer_orig, NULL);
    uint8_t overflow = 0;

    switch(multi_converter_state->unit_type_orig) {
    default:
        break;
    case UnitTypeKilometers:
        if(multi_converter_state->unit_type_dest == UnitTypeMeters)
            a *= ((double)1000);
        else if(multi_converter_state->unit_type_dest == UnitTypeCentimeters)
            a *= ((double)100000);
        else if(multi_converter_state->unit_type_dest == UnitTypeMiles)
            a *= ((double)0.6213711);
        else if(multi_converter_state->unit_type_dest == UnitTypeFeet)
            a *= ((double)3280.839895013);
        else if(multi_converter_state->unit_type_dest == UnitTypeInches)
            a *= ((double)39370.078740157);
        break;
    case UnitTypeMeters:
        if(multi_converter_state->unit_type_dest == UnitTypeKilometers)
            a /= ((double)1000);
        else if(multi_converter_state->unit_type_dest == UnitTypeCentimeters)
            a *= ((double)100);
        else if(multi_converter_state->unit_type_dest == UnitTypeMiles)
            a *= ((double)0.0006213711);
        else if(multi_converter_state->unit_type_dest == UnitTypeFeet)
            a *= ((double)3.280839895013);
        else if(multi_converter_state->unit_type_dest == UnitTypeInches)
            a *= ((double)39.370078740157);
        break;
    case UnitTypeCentimeters:
        if(multi_converter_state->unit_type_dest == UnitTypeKilometers)
            a /= ((double)100000);
        else if(multi_converter_state->unit_type_dest == UnitTypeMeters)
            a /= ((double)100);
        else if(multi_converter_state->unit_type_dest == UnitTypeMiles)
            a *= ((double)0.000006213711);
        else if(multi_converter_state->unit_type_dest == UnitTypeFeet)
            a *= ((double)0.03280839895013);
        else if(multi_converter_state->unit_type_dest == UnitTypeInches)
            a *= ((double)0.39370078740157);
        break;

    case UnitTypeMiles:
        if(multi_converter_state->unit_type_dest == UnitTypeKilometers)
            a *= ((double)1.609344);
        else if(multi_converter_state->unit_type_dest == UnitTypeMeters)
            a *= ((double)1609.344);
        else if(multi_converter_state->unit_type_dest == UnitTypeCentimeters)
            a *= ((double)160934.4);
        else if(multi_converter_state->unit_type_dest == UnitTypeFeet)
            a *= ((double)5280);
        else if(multi_converter_state->unit_type_dest == UnitTypeInches)
            a *= ((double)63360);
        break;
    case UnitTypeFeet:
        if(multi_converter_state->unit_type_dest == UnitTypeKilometers)
            a *= ((double)0.0003048);
        else if(multi_converter_state->unit_type_dest == UnitTypeMeters)
            a *= ((double)0.3048);
        else if(multi_converter_state->unit_type_dest == UnitTypeCentimeters)
            a *= ((double)30.48);
        else if(multi_converter_state->unit_type_dest == UnitTypeMiles)
            a *= ((double)0.000189393939394);
        else if(multi_converter_state->unit_type_dest == UnitTypeInches)
            a *= ((double)12);
        break;
    case UnitTypeInches:
        if(multi_converter_state->unit_type_dest == UnitTypeKilometers)
            a *= ((double)0.0000254);
        else if(multi_converter_state->unit_type_dest == UnitTypeMeters)
            a *= ((double)0.0254);
        else if(multi_converter_state->unit_type_dest == UnitTypeCentimeters)
            a *= ((double)2.54);
        else if(multi_converter_state->unit_type_dest == UnitTypeMiles)
            a *= ((double)0.0000157828282828);
        else if(multi_converter_state->unit_type_dest == UnitTypeFeet)
            a *= ((double)0.0833333333333);
        break;
    }

    if(overflow) {
        multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    } else {
        int ret = snprintf(
            multi_converter_state->buffer_dest, MULTI_CONVERTER_NUMBER_DIGITS + 1, "%lf", a);

        if(ret < 0) multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    }
}

uint8_t multi_converter_unit_distance_allowed(MultiConverterUnitType unit_type) {
    return (
        unit_type == UnitTypeKilometers || unit_type == UnitTypeMeters ||
        unit_type == UnitTypeCentimeters || unit_type == UnitTypeMiles ||
        unit_type == UnitTypeFeet || unit_type == UnitTypeInches);
}

//
// DEG / RAD
//

void multi_converter_unit_angle_convert(MultiConverterState* const multi_converter_state) {
    double a = strtof(multi_converter_state->buffer_orig, NULL);
    uint8_t overflow = 0;

    switch(multi_converter_state->unit_type_orig) {
    default:
        break;
    case UnitTypeDegree:
        if(multi_converter_state->unit_type_dest == UnitTypeRadian) a *= ((double)0.0174532925199);
        break;

    case UnitTypeRadian:
        if(multi_converter_state->unit_type_dest == UnitTypeDegree) a *= ((double)57.2957795131);
        break;
    }

    if(overflow) {
        multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    } else {
        int ret = snprintf(
            multi_converter_state->buffer_dest, MULTI_CONVERTER_NUMBER_DIGITS + 1, "%lf", a);

        if(ret < 0) multi_converter_unit_set_overflow(multi_converter_state->buffer_dest);
    }
}

uint8_t multi_converter_unit_angle_allowed(MultiConverterUnitType unit_type) {
    return (unit_type == UnitTypeDegree || unit_type == UnitTypeRadian);
}