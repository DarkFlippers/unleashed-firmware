#include "simple_array.h"

#include <furi.h>

struct SimpleArray {
    const SimpleArrayConfig* config;
    SimpleArrayElement* data;
    uint32_t count;
};

SimpleArray* simple_array_alloc(const SimpleArrayConfig* config) {
    SimpleArray* instance = malloc(sizeof(SimpleArray));
    instance->config = config;
    return instance;
}

void simple_array_free(SimpleArray* instance) {
    furi_check(instance);

    simple_array_reset(instance);
    free(instance);
}

void simple_array_init(SimpleArray* instance, uint32_t count) {
    furi_check(instance);
    furi_check(count > 0);

    simple_array_reset(instance);

    instance->data = malloc(count * instance->config->type_size);
    instance->count = count;

    SimpleArrayInit init = instance->config->init;
    if(init) {
        for(uint32_t i = 0; i < instance->count; ++i) {
            init(simple_array_get(instance, i));
        }
    }
}

void simple_array_reset(SimpleArray* instance) {
    furi_check(instance);

    if(instance->data) {
        SimpleArrayReset reset = instance->config->reset;

        if(reset) {
            for(uint32_t i = 0; i < instance->count; ++i) {
                reset(simple_array_get(instance, i));
            }
        }

        free(instance->data);

        instance->count = 0;
        instance->data = NULL;
    }
}

void simple_array_copy(SimpleArray* instance, const SimpleArray* other) {
    furi_check(instance);
    furi_check(other);
    furi_check(instance->config == other->config);

    simple_array_reset(instance);

    if(other->count == 0) {
        return;
    }

    simple_array_init(instance, other->count);

    SimpleArrayCopy copy = instance->config->copy;
    if(copy) {
        for(uint32_t i = 0; i < other->count; ++i) {
            copy(simple_array_get(instance, i), simple_array_cget(other, i));
        }
    } else {
        memcpy(instance->data, other->data, other->count * instance->config->type_size);
    }
}

bool simple_array_is_equal(const SimpleArray* instance, const SimpleArray* other) {
    furi_check(instance);
    furi_check(other);

    // Equal if the same object
    if(instance == other) return true;

    return (instance->config == other->config) && (instance->count == other->count) &&
           ((instance->data == other->data) || (instance->data == NULL) || (other->data == NULL) ||
            (memcmp(instance->data, other->data, other->count) == 0));
}

uint32_t simple_array_get_count(const SimpleArray* instance) {
    furi_check(instance);
    return instance->count;
}

SimpleArrayElement* simple_array_get(SimpleArray* instance, uint32_t index) {
    furi_check(instance);
    furi_check(index < instance->count);

    return instance->data + index * instance->config->type_size;
}

const SimpleArrayElement* simple_array_cget(const SimpleArray* instance, uint32_t index) {
    furi_check(instance);
    return simple_array_get((SimpleArrayElement*)instance, index);
}

SimpleArrayData* simple_array_get_data(SimpleArray* instance) {
    furi_check(instance);
    furi_check(instance->data);

    return instance->data;
}

const SimpleArrayData* simple_array_cget_data(const SimpleArray* instance) {
    furi_check(instance);
    return simple_array_get_data((SimpleArray*)instance);
}

const SimpleArrayConfig simple_array_config_uint8_t = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(uint8_t),
};
