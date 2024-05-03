#include "varint_pair.h"
#include <toolbox/varint.h>

#define VARINT_PAIR_SIZE 10

struct VarintPair {
    size_t data_length;
    uint8_t data[VARINT_PAIR_SIZE];
};

VarintPair* varint_pair_alloc(void) {
    VarintPair* pair = malloc(sizeof(VarintPair));
    pair->data_length = 0;
    return pair;
}

void varint_pair_free(VarintPair* pair) {
    free(pair);
}

bool varint_pair_pack(VarintPair* pair, bool first, uint32_t value) {
    bool result = false;

    if(first) {
        if(pair->data_length == 0) {
            pair->data_length = varint_uint32_pack(value, pair->data);
        } else {
            pair->data_length = 0;
        }
    } else {
        if(pair->data_length != 0) {
            pair->data_length += varint_uint32_pack(value, pair->data + pair->data_length);
            result = true;
        }
    }

    return result;
}

bool varint_pair_unpack(
    uint8_t* data,
    size_t data_length,
    uint32_t* value_1,
    uint32_t* value_2,
    size_t* length) {
    size_t size = 0;
    uint32_t tmp_value_1;
    uint32_t tmp_value_2;

    size += varint_uint32_unpack(&tmp_value_1, &data[size], data_length);

    if(size >= data_length) {
        return false;
    }

    size += varint_uint32_unpack(&tmp_value_2, &data[size], (size_t)(data_length - size));

    *value_1 = tmp_value_1;
    *value_2 = tmp_value_2;
    *length = size;

    return true;
}

uint8_t* varint_pair_get_data(VarintPair* pair) {
    return pair->data;
}

size_t varint_pair_get_size(VarintPair* pair) {
    return pair->data_length;
}

void varint_pair_reset(VarintPair* pair) {
    pair->data_length = 0;
}
