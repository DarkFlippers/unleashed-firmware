#include "mosgortrans_util.h"

#define TAG "Mosgortrans"

void render_section_header(
    FuriString* str,
    const char* name,
    uint8_t prefix_separator_cnt,
    uint8_t suffix_separator_cnt) {
    for(uint8_t i = 0; i < prefix_separator_cnt; i++) {
        furi_string_cat_printf(str, ":");
    }
    furi_string_cat_printf(str, "[ %s ]", name);
    for(uint8_t i = 0; i < suffix_separator_cnt; i++) {
        furi_string_cat_printf(str, ":");
    }
}

void from_days_to_datetime(uint32_t days, DateTime* datetime, uint16_t start_year) {
    uint32_t timestamp = days * 24 * 60 * 60;
    DateTime start_datetime = {0};
    start_datetime.year = start_year - 1;
    start_datetime.month = 12;
    start_datetime.day = 31;
    timestamp += datetime_datetime_to_timestamp(&start_datetime);
    datetime_timestamp_to_datetime(timestamp, datetime);
}

void from_minutes_to_datetime(uint32_t minutes, DateTime* datetime, uint16_t start_year) {
    uint32_t timestamp = minutes * 60;
    DateTime start_datetime = {0};
    start_datetime.year = start_year - 1;
    start_datetime.month = 12;
    start_datetime.day = 31;
    timestamp += datetime_datetime_to_timestamp(&start_datetime);
    datetime_timestamp_to_datetime(timestamp, datetime);
}

void from_seconds_to_datetime(uint32_t seconds, DateTime* datetime, uint16_t start_year) {
    uint32_t timestamp = seconds;
    DateTime start_datetime = {0};
    start_datetime.year = start_year - 1;
    start_datetime.month = 12;
    start_datetime.day = 31;
    timestamp += datetime_datetime_to_timestamp(&start_datetime);
    datetime_timestamp_to_datetime(timestamp, datetime);
}

typedef struct {
    uint16_t view; //101
    uint16_t type; //102
    uint8_t layout; //111
    uint8_t layout2; //112
    uint16_t blank_type; //121
    uint16_t type_of_extended; //122
    uint8_t extended; //123
    uint8_t benefit_code; //124
    uint32_t number; //201
    uint16_t use_before_date; //202
    uint16_t use_before_date2; //202.2
    uint16_t use_with_date; //205
    uint8_t requires_activation; //301
    uint16_t activate_during; //302
    uint16_t extension_counter; //304
    uint8_t blocked; //303
    uint32_t valid_from_date; //311
    uint16_t valid_to_date; //312
    uint8_t valid_for_days; //313
    uint32_t valid_for_minutes; //314
    uint16_t valid_for_time; //316
    uint16_t valid_for_time2; //316.2
    uint32_t valid_to_time; //317
    uint16_t remaining_trips; //321
    uint8_t remaining_trips1; //321.1
    uint32_t remaining_funds; //322
    uint16_t total_trips; //331
    uint16_t start_trip_date; //402
    uint16_t start_trip_time; //403
    uint32_t start_trip_neg_minutes; //404
    uint32_t start_trip_minutes; //405
    uint8_t start_trip_seconds; //406
    uint8_t minutes_pass; //412
    uint8_t passage_5_minutes; //413
    uint8_t metro_ride_with; //414
    uint8_t transport_type; //421
    uint8_t transport_type_flag; //421.0
    uint8_t transport_type1; //421.1
    uint8_t transport_type2; //421.2
    uint8_t transport_type3; //421.3
    uint8_t transport_type4; //421.4
    uint16_t validator; //422
    uint8_t validator1; //422.1
    uint16_t validator2; //422.2
    uint16_t route; //424
    uint8_t passage_in_metro; //431
    uint8_t transfer_in_metro; //432
    uint8_t passages_ground_transport; //433
    uint8_t fare_trip; //441
    uint16_t crc16; //501.1
    uint16_t crc16_2; //501.2
    uint32_t hash; //502
    uint16_t hash1; //502.1
    uint32_t hash2; //502.2
    uint8_t geozone_a; //GeoZoneA
    uint8_t geozone_b; //GeoZoneB
    uint8_t company; //Company
    uint8_t units; //Units
    uint64_t rfu1; //rfu1
    uint16_t rfu2; //rfu2
    uint32_t rfu3; //rfu3
    uint8_t rfu4; //rfu4
    uint8_t rfu5; //rfu5
    uint8_t write_enabled; //write_enabled
    uint32_t tech_code; //TechCode
    uint8_t interval; //Interval
    uint16_t app_code1; //AppCode1
    uint16_t app_code2; //AppCode2
    uint16_t app_code3; //AppCode3
    uint16_t app_code4; //AppCode4
    uint16_t type1; //Type1
    uint16_t type2; //Type2
    uint16_t type3; //Type3
    uint16_t type4; //Type4
    uint8_t zoo; //zoo
} BlockData;

void parse_layout_2(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x38, 16); //202
    data_block->benefit_code = bit_lib_get_bits(block->data, 0x48, 8); //124
    data_block->rfu1 = bit_lib_get_bits_32(block->data, 0x50, 32); //rfu1
    data_block->crc16 = bit_lib_get_bits_16(block->data, 0x70, 16); //501.1
    data_block->blocked = bit_lib_get_bits(block->data, 0x80, 1); //303
    data_block->start_trip_time = bit_lib_get_bits_16(block->data, 0x81, 12); //403
    data_block->start_trip_date = bit_lib_get_bits_16(block->data, 0x8D, 16); //402
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x9D, 16); //311
    data_block->valid_to_date = bit_lib_get_bits_16(block->data, 0xAD, 16); //312
    data_block->start_trip_seconds = bit_lib_get_bits(block->data, 0xDB, 6); //406
    data_block->transport_type1 = bit_lib_get_bits(block->data, 0xC3, 2); //421.1
    data_block->transport_type2 = bit_lib_get_bits(block->data, 0xC5, 2); //421.2
    data_block->transport_type3 = bit_lib_get_bits(block->data, 0xC7, 2); //421.3
    data_block->transport_type4 = bit_lib_get_bits(block->data, 0xC9, 2); //421.4
    data_block->use_with_date = bit_lib_get_bits_16(block->data, 0xBD, 16); //205
    data_block->route = bit_lib_get_bits(block->data, 0xCD, 1); //424
    data_block->validator1 = bit_lib_get_bits_16(block->data, 0xCE, 15); //422.1
    data_block->validator = bit_lib_get_bits_16(block->data, 0xCD, 16);
    data_block->total_trips = bit_lib_get_bits_16(block->data, 0xDD, 16); //331
    data_block->write_enabled = bit_lib_get_bits(block->data, 0xED, 1); //write_enabled
    data_block->rfu2 = bit_lib_get_bits(block->data, 0xEE, 2); //rfu2
    data_block->crc16_2 = bit_lib_get_bits_16(block->data, 0xF0, 16); //501.2
}

void parse_layout_6(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x38, 16); //202
    data_block->geozone_a = bit_lib_get_bits(block->data, 0x48, 4); //GeoZoneA
    data_block->geozone_b = bit_lib_get_bits(block->data, 0x4C, 4); //GeoZoneB
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x50, 10); //121
    data_block->type_of_extended = bit_lib_get_bits_16(block->data, 0x5A, 10); //122
    data_block->rfu1 = bit_lib_get_bits_16(block->data, 0x64, 12); //rfu1
    data_block->crc16 = bit_lib_get_bits_16(block->data, 0x70, 16); //501.1
    data_block->blocked = bit_lib_get_bits(block->data, 0x80, 1); //303
    data_block->start_trip_time = bit_lib_get_bits_16(block->data, 0x81, 12); //403
    data_block->start_trip_date = bit_lib_get_bits_16(block->data, 0x8D, 16); //402
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x9D, 16); //311
    data_block->valid_to_date = bit_lib_get_bits_16(block->data, 0xAD, 16); //312
    data_block->company = bit_lib_get_bits(block->data, 0xBD, 4); //Company
    data_block->validator1 = bit_lib_get_bits(block->data, 0xC1, 4); //422.1
    data_block->remaining_trips = bit_lib_get_bits_16(block->data, 0xC5, 10); //321
    data_block->units = bit_lib_get_bits(block->data, 0xCF, 6); //Units
    data_block->validator2 = bit_lib_get_bits_16(block->data, 0xD5, 10); //422.2
    data_block->total_trips = bit_lib_get_bits_16(block->data, 0xDF, 16); //331
    data_block->extended = bit_lib_get_bits(block->data, 0xEF, 1); //123
    data_block->crc16_2 = bit_lib_get_bits_16(block->data, 0xF0, 16); //501.2
}

void parse_layout_8(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x38, 16); //202
    data_block->rfu1 = bit_lib_get_bits_64(block->data, 0x48, 56); //rfu1
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x80, 16); //311
    data_block->valid_for_days = bit_lib_get_bits(block->data, 0x90, 8); //313
    data_block->requires_activation = bit_lib_get_bits(block->data, 0x98, 1); //301
    data_block->rfu2 = bit_lib_get_bits(block->data, 0x99, 7); //rfu2
    data_block->remaining_trips1 = bit_lib_get_bits(block->data, 0xA0, 8); //321.1
    data_block->remaining_trips = bit_lib_get_bits(block->data, 0xA8, 8); //321
    data_block->validator1 = bit_lib_get_bits(block->data, 0xB0, 2); //422.1
    data_block->validator = bit_lib_get_bits_16(block->data, 0xB1, 15); //422
    data_block->hash = bit_lib_get_bits_32(block->data, 0xC0, 32); //502
    data_block->rfu3 = bit_lib_get_bits_32(block->data, 0xE0, 32); //rfu3
}

void parse_layout_A(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x40, 12); //311
    data_block->valid_for_minutes = bit_lib_get_bits_32(block->data, 0x4C, 19); //314
    data_block->requires_activation = bit_lib_get_bits(block->data, 0x5F, 1); //301
    data_block->start_trip_minutes = bit_lib_get_bits_32(block->data, 0x60, 19); //405
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0x77, 7); //412
    data_block->transport_type_flag = bit_lib_get_bits(block->data, 0x7E, 2); //421.0
    data_block->remaining_trips = bit_lib_get_bits(block->data, 0x80, 8); //321
    data_block->validator = bit_lib_get_bits_16(block->data, 0x88, 16); //422
    data_block->transport_type1 = bit_lib_get_bits(block->data, 0x98, 2); //421.1
    data_block->transport_type2 = bit_lib_get_bits(block->data, 0x9A, 2); //421.2
    data_block->transport_type3 = bit_lib_get_bits(block->data, 0x9C, 2); //421.3
    data_block->transport_type4 = bit_lib_get_bits(block->data, 0x9E, 2); //421.4
    data_block->hash = bit_lib_get_bits_32(block->data, 0xC0, 32); //502
}

void parse_layout_C(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x38, 16); //202
    data_block->rfu1 = bit_lib_get_bits_64(block->data, 0x48, 56); //rfu1
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x80, 16); //311
    data_block->valid_for_days = bit_lib_get_bits(block->data, 0x90, 8); //313
    data_block->requires_activation = bit_lib_get_bits(block->data, 0x98, 1); //301
    data_block->rfu2 = bit_lib_get_bits_16(block->data, 0x99, 13); //rfu2
    data_block->remaining_trips = bit_lib_get_bits_16(block->data, 0xA6, 10); //321
    data_block->validator = bit_lib_get_bits_16(block->data, 0xB0, 16); //422
    data_block->hash = bit_lib_get_bits_32(block->data, 0xC0, 32); //502
    data_block->start_trip_date = bit_lib_get_bits_16(block->data, 0xE0, 16); //402
    data_block->start_trip_time = bit_lib_get_bits_16(block->data, 0xF0, 11); //403
    data_block->transport_type = bit_lib_get_bits(block->data, 0xFB, 2); //421
    data_block->rfu3 = bit_lib_get_bits(block->data, 0xFD, 2); //rfu3
    data_block->transfer_in_metro = bit_lib_get_bits(block->data, 0xFF, 1); //432
}

void parse_layout_D(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->rfu1 = bit_lib_get_bits(block->data, 0x38, 8); //rfu1
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x40, 16); //202
    data_block->valid_for_time = bit_lib_get_bits_16(block->data, 0x50, 11); //316
    data_block->rfu2 = bit_lib_get_bits(block->data, 0x5B, 5); //rfu2
    data_block->use_before_date2 = bit_lib_get_bits_16(block->data, 0x60, 16); //202.2
    data_block->valid_for_time2 = bit_lib_get_bits_16(block->data, 0x70, 11); //316.2
    data_block->rfu3 = bit_lib_get_bits(block->data, 0x7B, 5); //rfu3
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x80, 16); //311
    data_block->valid_for_days = bit_lib_get_bits(block->data, 0x90, 8); //313
    data_block->requires_activation = bit_lib_get_bits(block->data, 0x98, 1); //301
    data_block->rfu4 = bit_lib_get_bits(block->data, 0x99, 2); //rfu4
    data_block->passage_5_minutes = bit_lib_get_bits(block->data, 0x9B, 5); //413
    data_block->transport_type1 = bit_lib_get_bits(block->data, 0xA0, 2); //421.1
    data_block->passage_in_metro = bit_lib_get_bits(block->data, 0xA2, 1); //431
    data_block->passages_ground_transport = bit_lib_get_bits(block->data, 0xA3, 3); //433
    data_block->remaining_trips = bit_lib_get_bits_16(block->data, 0xA6, 10); //321
    data_block->validator = bit_lib_get_bits_16(block->data, 0xB0, 16); //422
    data_block->hash = bit_lib_get_bits_32(block->data, 0xC0, 32); //502
    data_block->start_trip_date = bit_lib_get_bits_16(block->data, 0xE0, 16); //402
    data_block->start_trip_time = bit_lib_get_bits_16(block->data, 0xF0, 11); //403
    data_block->transport_type2 = bit_lib_get_bits(block->data, 0xFB, 2); //421.2
    data_block->rfu5 = bit_lib_get_bits(block->data, 0xFD, 2); //rfu5
    data_block->transfer_in_metro = bit_lib_get_bits(block->data, 0xFF, 1); //432
}

void parse_layout_E1(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->layout2 = bit_lib_get_bits(block->data, 0x38, 5); //112
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x3D, 16); //202
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x4D, 10); //121
    data_block->validator = bit_lib_get_bits_16(block->data, 0x80, 16); //422
    data_block->start_trip_date = bit_lib_get_bits_16(block->data, 0x90, 16); //402
    data_block->start_trip_time = bit_lib_get_bits_16(block->data, 0xA0, 11); //403
    data_block->transport_type1 = bit_lib_get_bits(block->data, 0xAB, 2); //421.1
    data_block->transport_type2 = bit_lib_get_bits(block->data, 0xAD, 2); //421.2
    data_block->transfer_in_metro = bit_lib_get_bits(block->data, 0xB1, 1); //432
    data_block->passage_in_metro = bit_lib_get_bits(block->data, 0xB2, 1); //431
    data_block->passages_ground_transport = bit_lib_get_bits(block->data, 0xB3, 3); //433
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0xB9, 8); //412
    data_block->remaining_funds = bit_lib_get_bits_32(block->data, 0xC4, 19); //322
    data_block->fare_trip = bit_lib_get_bits(block->data, 0xD7, 2); //441
    data_block->blocked = bit_lib_get_bits(block->data, 0x9D, 1); //303
    data_block->zoo = bit_lib_get_bits(block->data, 0xDA, 1); //zoo
    data_block->hash = bit_lib_get_bits_32(block->data, 0xE0, 32); //502
}

void parse_layout_E2(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->layout2 = bit_lib_get_bits(block->data, 0x38, 5); //112
    data_block->type_of_extended = bit_lib_get_bits_16(block->data, 0x3D, 10); //122
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x47, 16); //202
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x57, 10); //121
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x61, 16); //311
    data_block->activate_during = bit_lib_get_bits_16(block->data, 0x71, 9); //302
    data_block->valid_for_minutes = bit_lib_get_bits_32(block->data, 0x83, 20); //314
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0x9A, 8); //412
    data_block->transport_type = bit_lib_get_bits(block->data, 0xA3, 2); //421
    data_block->passage_in_metro = bit_lib_get_bits(block->data, 0xA5, 1); //431
    data_block->transfer_in_metro = bit_lib_get_bits(block->data, 0xA6, 1); //432
    data_block->remaining_trips = bit_lib_get_bits_16(block->data, 0xA7, 10); //321
    data_block->validator = bit_lib_get_bits_16(block->data, 0xB1, 16); //422
    data_block->start_trip_neg_minutes = bit_lib_get_bits_32(block->data, 0xC4, 20); //404
    data_block->requires_activation = bit_lib_get_bits(block->data, 0xD8, 1); //301
    data_block->blocked = bit_lib_get_bits(block->data, 0xD9, 1); //303
    data_block->extended = bit_lib_get_bits(block->data, 0xDA, 1); //123
    data_block->hash = bit_lib_get_bits_32(block->data, 0xE0, 32); //502
}

void parse_layout_E3(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->layout2 = bit_lib_get_bits(block->data, 0x38, 5); //112
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 61, 16); //202
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x4D, 10); //121
    data_block->remaining_funds = bit_lib_get_bits_32(block->data, 0xBC, 22); //322
    data_block->hash = bit_lib_get_bits_32(block->data, 224, 32); //502
    data_block->validator = bit_lib_get_bits_16(block->data, 0x80, 16); //422
    data_block->start_trip_minutes = bit_lib_get_bits_32(block->data, 0x90, 23); //405
    data_block->fare_trip = bit_lib_get_bits(block->data, 0xD2, 2); //441
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0xAB, 7); //412
    data_block->transport_type_flag = bit_lib_get_bits(block->data, 0xB2, 2); //421.0
    data_block->transport_type1 = bit_lib_get_bits(block->data, 0xB4, 2); //421.1
    data_block->transport_type2 = bit_lib_get_bits(block->data, 0xB6, 2); //421.2
    data_block->transport_type3 = bit_lib_get_bits(block->data, 0xB8, 2); //421.3
    data_block->transport_type4 = bit_lib_get_bits(block->data, 0xBA, 2); //421.4
    data_block->blocked = bit_lib_get_bits(block->data, 0xD4, 1); //303
}

void parse_layout_E4(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->layout2 = bit_lib_get_bits(block->data, 0x38, 5); //112
    data_block->type_of_extended = bit_lib_get_bits_16(block->data, 0x3D, 10); //122
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x47, 13); //202
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x54, 10); //121
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x5E, 13); //311
    data_block->activate_during = bit_lib_get_bits_16(block->data, 0x6B, 9); //302
    data_block->extension_counter = bit_lib_get_bits_16(block->data, 0x74, 10); //304
    data_block->valid_for_minutes = bit_lib_get_bits_32(block->data, 0x80, 20); //314
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0x98, 7); //412
    data_block->transport_type_flag = bit_lib_get_bits(block->data, 0x9F, 2); //421.0
    data_block->transport_type1 = bit_lib_get_bits(block->data, 0xA1, 2); //421.1
    data_block->transport_type2 = bit_lib_get_bits(block->data, 0xA3, 2); //421.2
    data_block->transport_type3 = bit_lib_get_bits(block->data, 0xA5, 2); //421.3
    data_block->transport_type4 = bit_lib_get_bits(block->data, 0xA7, 2); //421.4
    data_block->remaining_trips = bit_lib_get_bits_16(block->data, 0xA9, 10); //321
    data_block->validator = bit_lib_get_bits_16(block->data, 0xB3, 16); //422
    data_block->start_trip_neg_minutes = bit_lib_get_bits_32(block->data, 0xC3, 20); //404
    data_block->requires_activation = bit_lib_get_bits(block->data, 0xD7, 1); //301
    data_block->blocked = bit_lib_get_bits(block->data, 0xD8, 1); //303
    data_block->extended = bit_lib_get_bits(block->data, 0xD9, 1); //123
    data_block->hash = bit_lib_get_bits_32(block->data, 0xE0, 32); //502
}

void parse_layout_E5(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->layout2 = bit_lib_get_bits(block->data, 0x38, 5); //112
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x3D, 13); //202
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x4A, 10); //121
    data_block->valid_to_time = bit_lib_get_bits_32(block->data, 0x54, 23); //317
    data_block->extension_counter = bit_lib_get_bits_16(block->data, 0x6B, 10); //304
    data_block->start_trip_minutes = bit_lib_get_bits_32(block->data, 0x80, 23); //405
    data_block->metro_ride_with = bit_lib_get_bits(block->data, 0x97, 7); //414
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0x9E, 7); //412
    data_block->remaining_funds = bit_lib_get_bits_32(block->data, 0xA7, 19); //322
    data_block->validator = bit_lib_get_bits_16(block->data, 0xBA, 16); //422
    data_block->blocked = bit_lib_get_bits(block->data, 0xCA, 1); //303
    data_block->route = bit_lib_get_bits_16(block->data, 0xCC, 12); //424
    data_block->passages_ground_transport = bit_lib_get_bits(block->data, 0xD8, 7); //433
    data_block->hash = bit_lib_get_bits_32(block->data, 0xE0, 32); //502
}

void parse_layout_E6(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->layout2 = bit_lib_get_bits(block->data, 0x38, 5); //112
    data_block->type_of_extended = bit_lib_get_bits_16(block->data, 0x3D, 10); //122
    data_block->use_before_date = bit_lib_get_bits_16(block->data, 0x47, 13); //202
    data_block->blank_type = bit_lib_get_bits_16(block->data, 0x54, 10); //121
    data_block->valid_from_date = bit_lib_get_bits_32(block->data, 0x5E, 23); //311
    data_block->extension_counter = bit_lib_get_bits_16(block->data, 0x75, 10); //304
    data_block->valid_for_minutes = bit_lib_get_bits_32(block->data, 0x80, 20); //314
    data_block->start_trip_neg_minutes = bit_lib_get_bits_32(block->data, 0x94, 20); //404
    data_block->metro_ride_with = bit_lib_get_bits(block->data, 0xA8, 7); //414
    data_block->minutes_pass = bit_lib_get_bits(block->data, 0xAF, 7); //412
    data_block->remaining_trips = bit_lib_get_bits(block->data, 0xB6, 7); //321
    data_block->validator = bit_lib_get_bits_16(block->data, 0xBD, 16); //422
    data_block->blocked = bit_lib_get_bits(block->data, 0xCD, 1); //303
    data_block->extended = bit_lib_get_bits(block->data, 0xCE, 1); //123
    data_block->route = bit_lib_get_bits_16(block->data, 0xD4, 12); //424
    data_block->hash = bit_lib_get_bits_32(block->data, 0xE0, 32); //502
}

void parse_layout_FCB(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->tech_code = bit_lib_get_bits_32(block->data, 0x38, 10); //tech_code
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x42, 16); //311
    data_block->valid_to_date = bit_lib_get_bits_16(block->data, 0x52, 16); //312
    data_block->interval = bit_lib_get_bits(block->data, 0x62, 4); //interval
    data_block->app_code1 = bit_lib_get_bits_16(block->data, 0x66, 10); //app_code1
    data_block->hash1 = bit_lib_get_bits_16(block->data, 0x70, 16); //502.1
    data_block->type1 = bit_lib_get_bits_16(block->data, 0x80, 10); //type1
    data_block->app_code2 = bit_lib_get_bits_16(block->data, 0x8A, 10); //app_code2
    data_block->type2 = bit_lib_get_bits_16(block->data, 0x94, 10); //type2
    data_block->app_code3 = bit_lib_get_bits_16(block->data, 0x9E, 10); //app_code3
    data_block->type3 = bit_lib_get_bits_16(block->data, 0xA8, 10); //type3
    data_block->app_code4 = bit_lib_get_bits_16(block->data, 0xB2, 10); //app_code4
    data_block->type4 = bit_lib_get_bits_16(block->data, 0xBC, 10); //type4
    data_block->hash2 = bit_lib_get_bits_32(block->data, 0xE0, 32); //502.2
}

void parse_layout_F0B(BlockData* data_block, const MfClassicBlock* block) {
    data_block->view = bit_lib_get_bits_16(block->data, 0x00, 10); //101
    data_block->type = bit_lib_get_bits_16(block->data, 0x0A, 10); //102
    data_block->number = bit_lib_get_bits_32(block->data, 0x14, 32); //201
    data_block->layout = bit_lib_get_bits(block->data, 0x34, 4); //111
    data_block->tech_code = bit_lib_get_bits_32(block->data, 0x38, 10); //tech_code
    data_block->valid_from_date = bit_lib_get_bits_16(block->data, 0x42, 16); //311
    data_block->valid_to_date = bit_lib_get_bits_16(block->data, 0x52, 16); //312
    data_block->hash1 = bit_lib_get_bits_32(block->data, 0x70, 16); //502.1
}

void parse_transport_type(BlockData* data_block, FuriString* transport) {
    switch(data_block->transport_type_flag) {
    case 1:
        uint8_t transport_type =
            (data_block->transport_type1 || data_block->transport_type2 ||
             data_block->transport_type3 || data_block->transport_type4);
        switch(transport_type) {
        case 1:
            furi_string_cat(transport, "Metro");
            break;
        case 2:
            furi_string_cat(transport, "Monorail");
            break;
        case 3:
            furi_string_cat(transport, "MCC");
            break;
        default:
            furi_string_cat(transport, "Unknown");
            break;
        }
        break;
    case 2:
        furi_string_cat(transport, "Ground");
        break;
    default:
        furi_string_cat(transport, "");
        break;
    }
}

bool mosgortrans_parse_transport_block(const MfClassicBlock* block, FuriString* result) {
    BlockData data_block = {};
    const uint16_t valid_departments[] = {0x106, 0x108, 0x10A, 0x10E, 0x110, 0x117};
    uint16_t transport_department = bit_lib_get_bits_16(block->data, 0, 10);
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        furi_string_cat_printf(result, "Transport department: %x\n", transport_department);
    }
    bool department_valid = false;
    for(uint8_t i = 0; i < 6; i++) {
        if(transport_department == valid_departments[i]) {
            department_valid = true;
            break;
        }
    }
    if(!department_valid) {
        return false;
    }
    FURI_LOG_D(TAG, "Transport department: %x", transport_department);
    uint16_t layout_type = bit_lib_get_bits_16(block->data, 52, 4);
    if(layout_type == 0xE) {
        layout_type = bit_lib_get_bits_16(block->data, 52, 9);
    } else if(layout_type == 0xF) {
        layout_type = bit_lib_get_bits_16(block->data, 52, 14);
    }
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        furi_string_cat_printf(result, "Layout: %x\n", layout_type);
    }
    FURI_LOG_D(TAG, "Layout type %x", layout_type);
    switch(layout_type) {
    case 0x02: {
        parse_layout_2(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);

        if(data_block.valid_from_date == 0 || data_block.valid_to_date == 0) {
            furi_string_cat(result, "\e#No ticket");
            return false;
        }
        //remaining_trips
        furi_string_cat_printf(result, "Trips: %d\n", data_block.total_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        DateTime card_valid_to_date_s = {0};
        from_days_to_datetime(data_block.valid_to_date, &card_valid_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d\n",
            card_valid_to_date_s.day,
            card_valid_to_date_s.month,
            card_valid_to_date_s.year);
        //trip_number
        furi_string_cat_printf(result, "Trips: %d\n", data_block.total_trips);
        //trip_from
        DateTime card_start_trip_minutes_s = {0};
        from_seconds_to_datetime(
            data_block.start_trip_date * 24 * 60 * 60 + data_block.start_trip_time * 60 +
                data_block.start_trip_seconds,
            &card_start_trip_minutes_s,
            1992);
        furi_string_cat_printf(
            result,
            "Trip from: %02d.%02d.%04d %02d:%02d",
            card_start_trip_minutes_s.day,
            card_start_trip_minutes_s.month,
            card_start_trip_minutes_s.year,
            card_start_trip_minutes_s.hour,
            card_start_trip_minutes_s.minute);
        break;
    }
    case 0x06: {
        parse_layout_6(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        DateTime card_valid_to_date_s = {0};
        from_days_to_datetime(data_block.valid_to_date, &card_valid_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d\n",
            card_valid_to_date_s.day,
            card_valid_to_date_s.month,
            card_valid_to_date_s.year);
        //trip_number
        furi_string_cat_printf(result, "Trips: %d\n", data_block.total_trips);
        //trip_from
        DateTime card_start_trip_minutes_s = {0};
        from_minutes_to_datetime(
            (data_block.start_trip_date) * 24 * 60 + data_block.start_trip_time,
            &card_start_trip_minutes_s,
            1992);
        furi_string_cat_printf(
            result,
            "Trip from: %02d.%02d.%04d %02d:%02d\n",
            card_start_trip_minutes_s.day,
            card_start_trip_minutes_s.month,
            card_start_trip_minutes_s.year,
            card_start_trip_minutes_s.hour,
            card_start_trip_minutes_s.minute);
        //validator
        furi_string_cat_printf(
            result, "Validator: %05d", data_block.validator1 * 1024 + data_block.validator2);
        break;
    }
    case 0x08: {
        parse_layout_8(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        DateTime card_valid_to_date_s = {0};
        from_days_to_datetime(
            data_block.valid_from_date + data_block.valid_for_days, &card_valid_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d",
            card_valid_to_date_s.day,
            card_valid_to_date_s.month,
            card_valid_to_date_s.year);
        break;
    }
    case 0x0A: {
        parse_layout_A(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 2016);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 2016);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        DateTime card_valid_to_date_s = {0};
        from_minutes_to_datetime(
            data_block.valid_from_date * 24 * 60 + data_block.valid_for_minutes - 1,
            &card_valid_to_date_s,
            2016);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d",
            card_valid_to_date_s.day,
            card_valid_to_date_s.month,
            card_valid_to_date_s.year);
        //trip_from
        if(data_block.start_trip_minutes) {
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.valid_from_date * 24 * 60 + data_block.start_trip_minutes,
                &card_start_trip_minutes_s,
                2016);
            furi_string_cat_printf(
                result,
                "\nTrip from: %02d.%02d.%04d %02d:%02d",
                card_start_trip_minutes_s.day,
                card_start_trip_minutes_s.month,
                card_start_trip_minutes_s.year,
                card_start_trip_minutes_s.hour,
                card_start_trip_minutes_s.minute);
        }
        //trip_switch
        if(data_block.minutes_pass) {
            DateTime card_start_switch_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.valid_from_date * 24 * 60 + data_block.start_trip_minutes +
                    data_block.minutes_pass,
                &card_start_switch_trip_minutes_s,
                2016);
            furi_string_cat_printf(
                result,
                "\nTrip switch: %02d.%02d.%04d %02d:%02d",
                card_start_switch_trip_minutes_s.day,
                card_start_switch_trip_minutes_s.month,
                card_start_switch_trip_minutes_s.year,
                card_start_switch_trip_minutes_s.hour,
                card_start_switch_trip_minutes_s.minute);
        }
        //transport
        FuriString* transport = furi_string_alloc();
        parse_transport_type(&data_block, transport);
        furi_string_cat_printf(result, "\nTransport: %s", furi_string_get_cstr(transport));
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        furi_string_free(transport);
        break;
    }
    case 0x0C: {
        parse_layout_C(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        DateTime card_valid_to_date_s = {0};
        from_days_to_datetime(
            data_block.valid_from_date + data_block.valid_for_days, &card_valid_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d\n",
            card_valid_to_date_s.day,
            card_valid_to_date_s.month,
            card_valid_to_date_s.year);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d", data_block.remaining_trips);
        //trip_from
        if(data_block.start_trip_date) { // TODO: (-nofl) unused
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_date * 24 * 60 + data_block.start_trip_time,
                &card_start_trip_minutes_s,
                1992);
        }
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        break;
    }
    case 0x0D: {
        parse_layout_D(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        DateTime card_valid_to_date_s = {0};
        from_days_to_datetime(
            data_block.valid_from_date + data_block.valid_for_days, &card_valid_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d",
            card_valid_to_date_s.day,
            card_valid_to_date_s.month,
            card_valid_to_date_s.year);
        //trip_from
        if(data_block.start_trip_date) { // TODO: (-nofl) unused
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_date * 24 * 60 + data_block.start_trip_time,
                &card_start_trip_minutes_s,
                1992);
        }
        //trip_switch
        if(data_block.passage_5_minutes) { // TODO: (-nofl) unused
            DateTime card_start_switch_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_date * 24 * 60 + data_block.start_trip_time +
                    data_block.passage_5_minutes,
                &card_start_switch_trip_minutes_s,
                1992);
        }
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        break;
    }
    case 0xE1:
    case 0x1C1: {
        parse_layout_E1(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_funds
        furi_string_cat_printf(result, "Balance: %ld rub\n", data_block.remaining_funds / 100);
        //trip_from
        if(data_block.start_trip_date) {
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_date * 24 * 60 + data_block.start_trip_time,
                &card_start_trip_minutes_s,
                1992);
            furi_string_cat_printf(
                result,
                "Trip from: %02d.%02d.%04d %02d:%02d\n",
                card_start_trip_minutes_s.day,
                card_start_trip_minutes_s.month,
                card_start_trip_minutes_s.year,
                card_start_trip_minutes_s.hour,
                card_start_trip_minutes_s.minute);
        }
        //transport
        FuriString* transport = furi_string_alloc();
        switch(data_block.transport_type1) {
        case 1:
            switch(data_block.transport_type2) {
            case 1:
                furi_string_cat(transport, "Metro");
                break;
            case 2:
                furi_string_cat(transport, "Monorail");
                break;
            default:
                furi_string_cat(transport, "Unknown");
                break;
            }
            break;
        case 2:
            furi_string_cat(transport, "Ground");
            break;
        case 3:
            furi_string_cat(transport, "MCC");
            break;
        default:
            furi_string_cat(transport, "");
            break;
        }
        furi_string_cat_printf(result, "Transport: %s", furi_string_get_cstr(transport));
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        furi_string_free(transport);
        break;
    }
    case 0xE2:
    case 0x1C2: {
        parse_layout_E2(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_valid_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_valid_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d",
            card_valid_from_date_s.day,
            card_valid_from_date_s.month,
            card_valid_from_date_s.year);
        //valid_to_date
        if(data_block.activate_during) {
            DateTime card_valid_to_date_s = {0};
            from_days_to_datetime(
                data_block.valid_from_date + data_block.activate_during,
                &card_valid_to_date_s,
                1992);
            furi_string_cat_printf(
                result,
                "\nValid to: %02d.%02d.%04d",
                card_valid_to_date_s.day,
                card_valid_to_date_s.month,
                card_valid_to_date_s.year);
        } else {
            DateTime card_valid_to_date_s = {0};
            from_minutes_to_datetime(
                data_block.valid_from_date * 24 * 60 + data_block.valid_for_minutes - 1,
                &card_valid_to_date_s,
                1992);
            furi_string_cat_printf(
                result,
                "\nValid to: %02d.%02d.%04d",
                card_valid_to_date_s.day,
                card_valid_to_date_s.month,
                card_valid_to_date_s.year);
        }
        //trip_from
        if(data_block.start_trip_neg_minutes) {
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.valid_to_date * 24 * 60 + data_block.valid_for_minutes -
                    data_block.start_trip_neg_minutes,
                &card_start_trip_minutes_s,
                1992); //-time
            furi_string_cat_printf(
                result,
                "\nTrip from: %02d.%02d.%04d %02d:%02d",
                card_start_trip_minutes_s.day,
                card_start_trip_minutes_s.month,
                card_start_trip_minutes_s.year,
                card_start_trip_minutes_s.hour,
                card_start_trip_minutes_s.minute);
        }
        //trip_switch
        if(data_block.minutes_pass) {
            DateTime card_start_switch_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.valid_from_date * 24 * 60 + data_block.valid_for_minutes -
                    data_block.start_trip_neg_minutes + data_block.minutes_pass,
                &card_start_switch_trip_minutes_s,
                1992);
            furi_string_cat_printf(
                result,
                "\nTrip switch: %02d.%02d.%04d %02d:%02d",
                card_start_switch_trip_minutes_s.day,
                card_start_switch_trip_minutes_s.month,
                card_start_switch_trip_minutes_s.year,
                card_start_switch_trip_minutes_s.hour,
                card_start_switch_trip_minutes_s.minute);
        }
        //transport
        FuriString* transport = furi_string_alloc();
        switch(data_block.transport_type) { // TODO: (-nofl) unused
        case 1:
            furi_string_cat(transport, "Metro");
            break;
        case 2:
            furi_string_cat(transport, "Monorail");
            break;
        case 3:
            furi_string_cat(transport, "Ground");
            break;
        default:
            furi_string_cat(transport, "Unknown");
            break;
        }
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        furi_string_free(transport);
        break;
    }
    case 0xE3:
    case 0x1C3: {
        parse_layout_E3(&data_block, block);
        // number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        // use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        // remaining_funds
        furi_string_cat_printf(result, "Balance: %lu rub\n", data_block.remaining_funds);
        // start_trip_minutes
        DateTime card_start_trip_minutes_s = {0};
        from_minutes_to_datetime(data_block.start_trip_minutes, &card_start_trip_minutes_s, 2016);
        furi_string_cat_printf(
            result,
            "Trip from: %02d.%02d.%04d %02d:%02d\n",
            card_start_trip_minutes_s.day,
            card_start_trip_minutes_s.month,
            card_start_trip_minutes_s.year,
            card_start_trip_minutes_s.hour,
            card_start_trip_minutes_s.minute);
        // transport
        FuriString* transport = furi_string_alloc();
        parse_transport_type(&data_block, transport);
        furi_string_cat_printf(result, "Transport: %s\n", furi_string_get_cstr(transport));
        // validator
        furi_string_cat_printf(result, "Validator: %05d\n", data_block.validator);
        // fare
        FuriString* fare = furi_string_alloc();
        switch(data_block.fare_trip) {
        case 0:
            furi_string_cat(fare, "");
            break;
        case 1:
            furi_string_cat(fare, "Single");
            break;
        case 2:
            furi_string_cat(fare, "90 minutes");
            break;
        default:
            furi_string_cat(fare, "Unknown");
            break;
        }
        furi_string_cat_printf(result, "Fare: %s", furi_string_get_cstr(fare));
        furi_string_free(fare);
        furi_string_free(transport);
        break;
    }
    case 0xE4:
    case 0x1C4: {
        parse_layout_E4(&data_block, block);

        // number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        // use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 2016);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        // remaining_funds
        furi_string_cat_printf(result, "Balance: %lu rub\n", data_block.remaining_funds);
        // valid_from_date
        DateTime card_use_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_use_from_date_s, 2016);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_use_from_date_s.day,
            card_use_from_date_s.month,
            card_use_from_date_s.year);
        // valid_to_date
        DateTime card_use_to_date_s = {0};
        if(data_block.requires_activation) {
            from_days_to_datetime(
                data_block.valid_from_date + data_block.activate_during,
                &card_use_to_date_s,
                2016);
        } else {
            from_minutes_to_datetime(
                data_block.valid_from_date * 24 * 60 + data_block.valid_for_minutes - 1,
                &card_use_to_date_s,
                2016);
        }

        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d\n",
            card_use_to_date_s.day,
            card_use_to_date_s.month,
            card_use_to_date_s.year);
        // trip_number
        // furi_string_cat_printf(result, "Trips left: %d", data_block.remaining_trips);
        // trip_from
        DateTime card_start_trip_minutes_s = {0};
        from_minutes_to_datetime(
            data_block.valid_from_date * 24 * 60 + data_block.valid_for_minutes -
                data_block.start_trip_neg_minutes,
            &card_start_trip_minutes_s,
            2016); // TODO: (-nofl) unused
        //transport
        FuriString* transport = furi_string_alloc();
        parse_transport_type(&data_block, transport);
        furi_string_cat_printf(result, "Transport: %s", furi_string_get_cstr(transport));
        // validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        furi_string_free(transport);
        break;
    }
    case 0xE5:
    case 0x1C5: {
        parse_layout_E5(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 2019);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_funds
        furi_string_cat_printf(result, "Balance: %ld rub", data_block.remaining_funds / 100);
        //start_trip_minutes
        if(data_block.start_trip_minutes) {
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_minutes, &card_start_trip_minutes_s, 2019);
            furi_string_cat_printf(
                result,
                "\nTrip from: %02d.%02d.%04d %02d:%02d",
                card_start_trip_minutes_s.day,
                card_start_trip_minutes_s.month,
                card_start_trip_minutes_s.year,
                card_start_trip_minutes_s.hour,
                card_start_trip_minutes_s.minute);
        }
        //start_m_trip_minutes
        if(data_block.metro_ride_with) {
            DateTime card_start_m_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_minutes + data_block.metro_ride_with,
                &card_start_m_trip_minutes_s,
                2019);
            furi_string_cat_printf(
                result,
                "\n(M) from: %02d.%02d.%04d %02d:%02d",
                card_start_m_trip_minutes_s.day,
                card_start_m_trip_minutes_s.month,
                card_start_m_trip_minutes_s.year,
                card_start_m_trip_minutes_s.hour,
                card_start_m_trip_minutes_s.minute);
        }
        if(data_block.minutes_pass) {
            DateTime card_start_change_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.start_trip_minutes + data_block.minutes_pass,
                &card_start_change_trip_minutes_s,
                2019);
            furi_string_cat_printf(
                result,
                "\nTrip edit: %02d.%02d.%04d %02d:%02d",
                card_start_change_trip_minutes_s.day,
                card_start_change_trip_minutes_s.month,
                card_start_change_trip_minutes_s.year,
                card_start_change_trip_minutes_s.hour,
                card_start_change_trip_minutes_s.minute);
        }
        //transport
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        break;
    }
    case 0xE6:
    case 0x1C6: {
        parse_layout_E6(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //use_before_date
        DateTime card_use_before_date_s = {0};
        from_days_to_datetime(data_block.use_before_date, &card_use_before_date_s, 2019);
        furi_string_cat_printf(
            result,
            "Use before: %02d.%02d.%04d\n",
            card_use_before_date_s.day,
            card_use_before_date_s.month,
            card_use_before_date_s.year);
        //remaining_trips
        furi_string_cat_printf(result, "Trips left: %d\n", data_block.remaining_trips);
        //valid_from_date
        DateTime card_use_from_date_s = {0};
        from_minutes_to_datetime(data_block.valid_from_date, &card_use_from_date_s, 2019);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_use_from_date_s.day,
            card_use_from_date_s.month,
            card_use_from_date_s.year);
        //valid_to_date
        DateTime card_use_to_date_s = {0};
        from_minutes_to_datetime(
            data_block.valid_from_date + data_block.valid_for_minutes - 1,
            &card_use_to_date_s,
            2019);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d",
            card_use_to_date_s.day,
            card_use_to_date_s.month,
            card_use_to_date_s.year);
        //start_trip_minutes
        if(data_block.start_trip_neg_minutes) {
            DateTime card_start_trip_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.valid_from_date + data_block.valid_for_minutes -
                    data_block.start_trip_neg_minutes,
                &card_start_trip_minutes_s,
                2019); //-time
            furi_string_cat_printf(
                result,
                "\nTrip from: %02d.%02d.%04d %02d:%02d",
                card_start_trip_minutes_s.day,
                card_start_trip_minutes_s.month,
                card_start_trip_minutes_s.year,
                card_start_trip_minutes_s.hour,
                card_start_trip_minutes_s.minute);
        }
        //start_trip_m_minutes
        if(data_block.metro_ride_with) {
            DateTime card_start_trip_m_minutes_s = {0};
            from_minutes_to_datetime(
                data_block.valid_from_date + data_block.valid_for_minutes -
                    data_block.start_trip_neg_minutes + data_block.metro_ride_with,
                &card_start_trip_m_minutes_s,
                2019);
            furi_string_cat_printf(
                result,
                "\n(M) from: %02d.%02d.%04d %02d:%02d",
                card_start_trip_m_minutes_s.day,
                card_start_trip_m_minutes_s.month,
                card_start_trip_m_minutes_s.year,
                card_start_trip_m_minutes_s.hour,
                card_start_trip_m_minutes_s.minute);
        }
        //transport
        //validator
        if(data_block.validator) {
            furi_string_cat_printf(result, "\nValidator: %05d", data_block.validator);
        }
        break;
    }
    case 0x3CCB: {
        parse_layout_FCB(&data_block, block);
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //valid_from_date
        DateTime card_use_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_use_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_use_from_date_s.day,
            card_use_from_date_s.month,
            card_use_from_date_s.year);
        //valid_to_date
        DateTime card_use_to_date_s = {0};
        from_days_to_datetime(data_block.valid_to_date, &card_use_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d",
            card_use_to_date_s.day,
            card_use_to_date_s.month,
            card_use_to_date_s.year);
        break;
    }
    case 0x3C0B: {
        //number
        furi_string_cat_printf(result, "Number: %010lu\n", data_block.number);
        //valid_from_date
        DateTime card_use_from_date_s = {0};
        from_days_to_datetime(data_block.valid_from_date, &card_use_from_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid from: %02d.%02d.%04d\n",
            card_use_from_date_s.day,
            card_use_from_date_s.month,
            card_use_from_date_s.year);
        //valid_to_date
        DateTime card_use_to_date_s = {0};
        from_days_to_datetime(data_block.valid_to_date, &card_use_to_date_s, 1992);
        furi_string_cat_printf(
            result,
            "Valid to: %02d.%02d.%04d",
            card_use_to_date_s.day,
            card_use_to_date_s.month,
            card_use_to_date_s.year);
        break;
    }
    default:
        result = NULL;
        return false;
    }

    return true;
}
