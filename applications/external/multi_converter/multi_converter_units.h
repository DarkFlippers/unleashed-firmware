#pragma once

#include <input/input.h>
#include <gui/gui.h>

#include "multi_converter_definitions.h"

#define MULTI_CONVERTER_AVAILABLE_UNITS 14

#define multi_converter_get_unit(unit_type) multi_converter_available_units[unit_type]
#define multi_converter_get_unit_type_offset(unit_type, offset)                                   \
    (((unit_type + offset) % MULTI_CONVERTER_AVAILABLE_UNITS + MULTI_CONVERTER_AVAILABLE_UNITS) % \
     MULTI_CONVERTER_AVAILABLE_UNITS)
// the modulo operation will fail with extremely large values on the units array

// DEC / HEX / BIN
void multi_converter_unit_dec_hex_bin_convert(MultiConverterState* const multi_converter_state);
uint8_t multi_converter_unit_dec_hex_bin_allowed(MultiConverterUnitType);

// CEL / FAR / KEL
void multi_converter_unit_temperature_convert(MultiConverterState* const multi_converter_state);
uint8_t multi_converter_unit_temperature_allowed(MultiConverterUnitType);

// KM / M / CM / MILES / FEET / INCHES
void multi_converter_unit_distance_convert(MultiConverterState* const multi_converter_state);
uint8_t multi_converter_unit_distance_allowed(MultiConverterUnitType);

// DEG / RAD
void multi_converter_unit_angle_convert(MultiConverterState* const multi_converter_state);
uint8_t multi_converter_unit_angle_allowed(MultiConverterUnitType unit_type);

//
// each unit is made of comma? + negative? + keyboard_length + mini_name + name + convert function + allowed function
// (setting functions as NULL will cause convert / select options to be ignored)
//
static const MultiConverterUnit multi_converter_unit_dec = {
    0,
    0,
    10,
    "DEC\0",
    "Decimal\0",
    multi_converter_unit_dec_hex_bin_convert,
    multi_converter_unit_dec_hex_bin_allowed};
static const MultiConverterUnit multi_converter_unit_hex = {
    0,
    0,
    16,
    "HEX\0",
    "Hexadecimal\0",
    multi_converter_unit_dec_hex_bin_convert,
    multi_converter_unit_dec_hex_bin_allowed};
static const MultiConverterUnit multi_converter_unit_bin = {
    0,
    0,
    2,
    "BIN\0",
    "Binary\0",
    multi_converter_unit_dec_hex_bin_convert,
    multi_converter_unit_dec_hex_bin_allowed};

static const MultiConverterUnit multi_converter_unit_cel = {
    1,
    1,
    10,
    "CEL\0",
    "Celsius\0",
    multi_converter_unit_temperature_convert,
    multi_converter_unit_temperature_allowed};
static const MultiConverterUnit multi_converter_unit_far = {
    1,
    1,
    10,
    "FAR\0",
    "Fahernheit\0",
    multi_converter_unit_temperature_convert,
    multi_converter_unit_temperature_allowed};
static const MultiConverterUnit multi_converter_unit_kel = {
    1,
    1,
    10,
    "KEL\0",
    "Kelvin\0",
    multi_converter_unit_temperature_convert,
    multi_converter_unit_temperature_allowed};

static const MultiConverterUnit multi_converter_unit_km = {
    1,
    0,
    10,
    "KM\0",
    "Kilometers\0",
    multi_converter_unit_distance_convert,
    multi_converter_unit_distance_allowed};
static const MultiConverterUnit multi_converter_unit_m = {
    1,
    0,
    10,
    "M\0",
    "Meters\0",
    multi_converter_unit_distance_convert,
    multi_converter_unit_distance_allowed};
static const MultiConverterUnit multi_converter_unit_cm = {
    1,
    0,
    10,
    "CM\0",
    "Centimeters\0",
    multi_converter_unit_distance_convert,
    multi_converter_unit_distance_allowed};
static const MultiConverterUnit multi_converter_unit_mi = {
    1,
    0,
    10,
    "MI\0",
    "Miles\0",
    multi_converter_unit_distance_convert,
    multi_converter_unit_distance_allowed};
static const MultiConverterUnit multi_converter_unit_ft = {
    1,
    0,
    10,
    "FT\0",
    "Feet\0",
    multi_converter_unit_distance_convert,
    multi_converter_unit_distance_allowed};
static const MultiConverterUnit multi_converter_unit_in = {
    1,
    0,
    10,
    " \"\0",
    "Inches\0",
    multi_converter_unit_distance_convert,
    multi_converter_unit_distance_allowed};

static const MultiConverterUnit multi_converter_unit_deg = {
    1,
    0,
    10,
    "DEG\0",
    "Degree\0",
    multi_converter_unit_angle_convert,
    multi_converter_unit_angle_allowed};
static const MultiConverterUnit multi_converter_unit_rad = {
    1,
    0,
    10,
    "RAD\0",
    "Radian\0",
    multi_converter_unit_angle_convert,
    multi_converter_unit_angle_allowed};

// index order set by the MultiConverterUnitType enum element (multi_converter_definitions.h)
static const MultiConverterUnit multi_converter_available_units[MULTI_CONVERTER_AVAILABLE_UNITS] = {
    [UnitTypeDec] = multi_converter_unit_dec,
    [UnitTypeHex] = multi_converter_unit_hex,
    [UnitTypeBin] = multi_converter_unit_bin,

    [UnitTypeCelsius] = multi_converter_unit_cel,
    [UnitTypeFahernheit] = multi_converter_unit_far,
    [UnitTypeKelvin] = multi_converter_unit_kel,

    [UnitTypeKilometers] = multi_converter_unit_km,
    [UnitTypeMeters] = multi_converter_unit_m,
    [UnitTypeCentimeters] = multi_converter_unit_cm,
    [UnitTypeMiles] = multi_converter_unit_mi,
    [UnitTypeFeet] = multi_converter_unit_ft,
    [UnitTypeInches] = multi_converter_unit_in,

    [UnitTypeDegree] = multi_converter_unit_deg,
    [UnitTypeRadian] = multi_converter_unit_rad,
};