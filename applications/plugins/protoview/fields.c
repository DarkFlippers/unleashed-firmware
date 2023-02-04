/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * Protocol fields implementation. */

#include "app.h"

/* Create a new field of the specified type. Without populating its
 * type-specific value. */
static ProtoViewField* field_new(ProtoViewFieldType type, const char* name) {
    ProtoViewField* f = malloc(sizeof(*f));
    f->type = type;
    f->name = strdup(name);
    return f;
}

/* Free only the auxiliary data of a field, used to represent the
 * current type. Name and type are not touched. */
static void field_free_aux_data(ProtoViewField* f) {
    switch(f->type) {
    case FieldTypeStr:
        free(f->str);
        break;
    case FieldTypeBytes:
        free(f->bytes);
        break;
    default:
        break; // Nothing to free for other types.
    }
}

/* Free a field an associated data. */
static void field_free(ProtoViewField* f) {
    field_free_aux_data(f);
    free(f->name);
    free(f);
}

/* Return the type of the field as string. */
const char* field_get_type_name(ProtoViewField* f) {
    switch(f->type) {
    case FieldTypeStr:
        return "str";
    case FieldTypeSignedInt:
        return "int";
    case FieldTypeUnsignedInt:
        return "uint";
    case FieldTypeBinary:
        return "bin";
    case FieldTypeHex:
        return "hex";
    case FieldTypeBytes:
        return "bytes";
    case FieldTypeFloat:
        return "float";
    }
    return "unknown";
}

/* Set a string representation of the specified field in buf. */
int field_to_string(char* buf, size_t len, ProtoViewField* f) {
    switch(f->type) {
    case FieldTypeStr:
        return snprintf(buf, len, "%s", f->str);
    case FieldTypeSignedInt:
        return snprintf(buf, len, "%lld", (long long)f->value);
    case FieldTypeUnsignedInt:
        return snprintf(buf, len, "%llu", (unsigned long long)f->uvalue);
    case FieldTypeBinary: {
        uint64_t test_bit = (1 << (f->len - 1));
        uint64_t idx = 0;
        while(idx < len - 1 && test_bit) {
            buf[idx++] = (f->uvalue & test_bit) ? '1' : '0';
            test_bit >>= 1;
        }
        buf[idx] = 0;
        return idx;
    }
    case FieldTypeHex:
        return snprintf(buf, len, "%*llX", (int)(f->len + 7) / 8, f->uvalue);
    case FieldTypeFloat:
        return snprintf(buf, len, "%.*f", (int)f->len, (double)f->fvalue);
    case FieldTypeBytes: {
        uint64_t idx = 0;
        while(idx < len - 1 && idx < f->len) {
            const char* charset = "0123456789ABCDEF";
            uint32_t nibble = idx & 1 ? (f->bytes[idx / 2] & 0xf) : (f->bytes[idx / 2] >> 4);
            buf[idx++] = charset[nibble];
        }
        buf[idx] = 0;
        return idx;
    }
    }
    return 0;
}

/* Set the field value from its string representation in 'buf'.
 * The field type must already be set and the field should be valid.
 * The string represenation 'buf' must be null termianted. Note that
 * even when representing binary values containing zero, this values
 * are taken as representations, so that would be the string "00" as
 * the Bytes type representation.
 *
 * The function returns true if the filed was successfully set to the
 * new value, otherwise if the specified value is invalid for the
 * field type, false is returned. */
bool field_set_from_string(ProtoViewField* f, char* buf, size_t len) {
    // Initialize values to zero since the Flipper sscanf() implementation
    // is fuzzy... may populate only part of the value.
    long long val = 0;
    unsigned long long uval = 0;
    float fval = 0;

    switch(f->type) {
    case FieldTypeStr:
        free(f->str);
        f->len = len;
        f->str = malloc(len + 1);
        memcpy(f->str, buf, len + 1);
        break;
    case FieldTypeSignedInt:
        if(!sscanf(buf, "%lld", &val)) return false;
        f->value = val;
        break;
    case FieldTypeUnsignedInt:
        if(!sscanf(buf, "%llu", &uval)) return false;
        f->uvalue = uval;
        break;
    case FieldTypeBinary: {
        uint64_t bit_to_set = (1 << (len - 1));
        uint64_t idx = 0;
        uval = 0;
        while(buf[idx]) {
            if(buf[idx] == '1')
                uval |= bit_to_set;
            else if(buf[idx] != '0')
                return false;
            bit_to_set >>= 1;
            idx++;
        }
        f->uvalue = uval;
    } break;
    case FieldTypeHex:
        if(!sscanf(buf, "%llx", &uval) && !sscanf(buf, "%llX", &uval)) return false;
        f->uvalue = uval;
        break;
    case FieldTypeFloat:
        if(!sscanf(buf, "%f", &fval)) return false;
        f->fvalue = fval;
        break;
    case FieldTypeBytes: {
        if(len > f->len) return false;
        uint64_t idx = 0;
        while(buf[idx]) {
            uint8_t nibble = 0;
            char c = toupper(buf[idx]);
            if(c >= '0' && c <= '9')
                nibble = c - '0';
            else if(c >= 'A' && c <= 'F')
                nibble = 10 + (c - 'A');
            else
                return false;

            if(idx & 1) {
                f->bytes[idx / 2] = (f->bytes[idx / 2] & 0xF0) | nibble;
            } else {
                f->bytes[idx / 2] = (f->bytes[idx / 2] & 0x0F) | (nibble << 4);
            }
            idx++;
        }
        buf[idx] = 0;
    } break;
    }
    return true;
}

/* Set the 'dst' field to contain a copy of the value of the 'src'
 * field. The field name is not modified. */
void field_set_from_field(ProtoViewField* dst, ProtoViewField* src) {
    field_free_aux_data(dst);
    dst->type = src->type;
    dst->len = src->len;
    switch(src->type) {
    case FieldTypeStr:
        dst->str = strdup(src->str);
        break;
    case FieldTypeBytes:
        dst->bytes = malloc(src->len);
        memcpy(dst->bytes, src->bytes, dst->len);
        break;
    case FieldTypeSignedInt:
        dst->value = src->value;
        break;
    case FieldTypeUnsignedInt:
    case FieldTypeBinary:
    case FieldTypeHex:
        dst->uvalue = src->uvalue;
        break;
    case FieldTypeFloat:
        dst->fvalue = src->fvalue;
        break;
    }
}

/* Increment the specified field value of 'incr'. If the field type
 * does not support increments false is returned, otherwise the
 * action is performed. */
bool field_incr_value(ProtoViewField* f, int incr) {
    switch(f->type) {
    case FieldTypeStr:
        return false;
    case FieldTypeSignedInt: {
        /* Wrap around depending on the number of bits (f->len)
             * the integer was declared to have. */
        int64_t max = (1ULL << (f->len - 1)) - 1;
        int64_t min = -max - 1;
        int64_t v = (int64_t)f->value + incr;
        if(v > max) v = min + (v - max - 1);
        if(v < min) v = max + (v - min + 1);
        f->value = v;
        break;
    }
    case FieldTypeBinary:
    case FieldTypeHex:
    case FieldTypeUnsignedInt: {
        /* Wrap around like for the unsigned case, but here
             * is simpler. */
        uint64_t max = (1ULL << f->len) - 1; // Broken for 64 bits.
        uint64_t uv = (uint64_t)f->value + incr;
        if(uv > max) uv = uv & max;
        f->uvalue = uv;
        break;
    }
    case FieldTypeFloat:
        f->fvalue += incr;
        break;
    case FieldTypeBytes: {
        // For bytes we only support single unit increments.
        if(incr != -1 && incr != 1) return false;
        for(int j = f->len - 1; j >= 0; j--) {
            uint8_t nibble = (j & 1) ? (f->bytes[j / 2] & 0x0F) : ((f->bytes[j / 2] & 0xF0) >> 4);

            nibble += incr;
            nibble &= 0x0F;

            f->bytes[j / 2] = (j & 1) ? ((f->bytes[j / 2] & 0xF0) | nibble) :
                                        ((f->bytes[j / 2] & 0x0F) | (nibble << 4));

            /* Propagate the operation on overflow of this nibble. */
            if((incr == 1 && nibble == 0) || (incr == -1 && nibble == 0xf)) {
                continue;
            }
            break; // Otherwise stop the loop here.
        }
        break;
    }
    }
    return true;
}

/* Free a field set and its contained fields. */
void fieldset_free(ProtoViewFieldSet* fs) {
    for(uint32_t j = 0; j < fs->numfields; j++) field_free(fs->fields[j]);
    free(fs->fields);
    free(fs);
}

/* Allocate and init an empty field set. */
ProtoViewFieldSet* fieldset_new(void) {
    ProtoViewFieldSet* fs = malloc(sizeof(*fs));
    fs->numfields = 0;
    fs->fields = NULL;
    return fs;
}

/* Append an already allocated field at the end of the specified field set. */
static void fieldset_add_field(ProtoViewFieldSet* fs, ProtoViewField* field) {
    fs->numfields++;
    fs->fields = realloc(fs->fields, sizeof(ProtoViewField*) * fs->numfields);
    fs->fields[fs->numfields - 1] = field;
}

/* Allocate and append an integer field. */
void fieldset_add_int(ProtoViewFieldSet* fs, const char* name, int64_t val, uint8_t bits) {
    ProtoViewField* f = field_new(FieldTypeSignedInt, name);
    f->value = val;
    f->len = bits;
    fieldset_add_field(fs, f);
}

/* Allocate and append an unsigned field. */
void fieldset_add_uint(ProtoViewFieldSet* fs, const char* name, uint64_t uval, uint8_t bits) {
    ProtoViewField* f = field_new(FieldTypeUnsignedInt, name);
    f->uvalue = uval;
    f->len = bits;
    fieldset_add_field(fs, f);
}

/* Allocate and append a hex field. This is an unsigned number but
 * with an hex representation. */
void fieldset_add_hex(ProtoViewFieldSet* fs, const char* name, uint64_t uval, uint8_t bits) {
    ProtoViewField* f = field_new(FieldTypeHex, name);
    f->uvalue = uval;
    f->len = bits;
    fieldset_add_field(fs, f);
}

/* Allocate and append a bin field. This is an unsigned number but
 * with a binary representation. */
void fieldset_add_bin(ProtoViewFieldSet* fs, const char* name, uint64_t uval, uint8_t bits) {
    ProtoViewField* f = field_new(FieldTypeBinary, name);
    f->uvalue = uval;
    f->len = bits;
    fieldset_add_field(fs, f);
}

/* Allocate and append a string field. The string 's' does not need to point
 * to a null terminated string, but must have at least 'len' valid bytes
 * starting from the pointer. The field object will be correctly null
 * terminated. */
void fieldset_add_str(ProtoViewFieldSet* fs, const char* name, const char* s, size_t len) {
    ProtoViewField* f = field_new(FieldTypeStr, name);
    f->len = len;
    f->str = malloc(len + 1);
    memcpy(f->str, s, len);
    f->str[len] = 0;
    fieldset_add_field(fs, f);
}

/* Allocate and append a bytes field. Note that 'count' is specified in
 * nibbles (bytes*2). */
void fieldset_add_bytes(
    ProtoViewFieldSet* fs,
    const char* name,
    const uint8_t* bytes,
    uint32_t count_nibbles) {
    uint32_t numbytes = (count_nibbles + count_nibbles % 2) / 2;
    ProtoViewField* f = field_new(FieldTypeBytes, name);
    f->bytes = malloc(numbytes);
    memcpy(f->bytes, bytes, numbytes);
    f->len = count_nibbles;
    fieldset_add_field(fs, f);
}

/* Allocate and append a float field. */
void fieldset_add_float(
    ProtoViewFieldSet* fs,
    const char* name,
    float val,
    uint32_t digits_after_dot) {
    ProtoViewField* f = field_new(FieldTypeFloat, name);
    f->fvalue = val;
    f->len = digits_after_dot;
    fieldset_add_field(fs, f);
}

/* For each field of the destination filedset 'dst', look for a matching
 * field name/type in the source fieldset 'src', and if one is found copy
 * its value into the 'dst' field. */
void fieldset_copy_matching_fields(ProtoViewFieldSet* dst, ProtoViewFieldSet* src) {
    for(uint32_t j = 0; j < dst->numfields; j++) {
        for(uint32_t i = 0; i < src->numfields; i++) {
            if(dst->fields[j]->type == src->fields[i]->type &&
               !strcmp(dst->fields[j]->name, src->fields[i]->name)) {
                field_set_from_field(dst->fields[j], src->fields[i]);
            }
        }
    }
}
