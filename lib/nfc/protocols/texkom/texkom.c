#include "texkom_i.h"

#include <furi.h>
#include <flipper_format.h>

#include <nfc/nfc_common.h>
#include <one_wire/maxim_crc.h>

#define TEXKOM_PROTOCOL_NAME "Texkom"
#define TEXKOM_TYPE_KEY      "Texkom Type"
#define TEXKOM_FRAME_KEY     "Frame"

#define TEXKOM_FRAME_BITS (TEXKOM_FRAME_SIZE * 8)

/** Raw bits a TK13-coded frame carries before pairs are collapsed. */
#define TEXKOM_TK13_RAW_BITS (TEXKOM_FRAME_BITS * 2)

/** Every variant opens the frame with FF FF. */
#define TEXKOM_PREAMBLE_BITS (16U)

typedef uint8_t (*TexkomCrcHandler)(const uint8_t* uid);

/**
 * @brief Everything that varies between variants, in one place.
 *
 * The per-variant frame layout is the whole content of this protocol, so it lives in one table
 * rather than being re-derived at each use.
 */
typedef struct {
    const char* full_name;
    const char* type_name;
    uint8_t uid_offset;
    uint8_t uid_size;
    TexkomCoding coding;
    /** Value of frame[2] that names this variant. Zero for Unknown, which names none. */
    uint8_t type_byte;
    /** NULL when the checksum algorithm is not known, which means it is left alone. */
    TexkomCrcHandler crc;
    /** Part of the checksum byte this variant owns. MMBIT gets only the high nibble. */
    uint8_t crc_mask;
} TexkomFeatures;

static uint8_t texkom_crc_tk13(const uint8_t* uid);
static uint8_t texkom_crc_tk17(const uint8_t* uid);
static uint8_t texkom_crc_mmbit(const uint8_t* uid);

static const TexkomFeatures texkom_features[TexkomTypeNum] = {
    [TexkomTypeUnknown] =
        {
            .type_byte = 0x00,
            .full_name = "Texkom (unknown)",
            .type_name = "Unknown",
            .uid_offset = TEXKOM_UID_OFFSET,
            .uid_size = TEXKOM_UID_SIZE,
            .coding = TexkomCodingTk13,
            .crc = NULL,
            .crc_mask = 0x00,
        },
    [TexkomTypeTk13] =
        {
            .type_byte = TEXKOM_TYPE_BYTE_TK13,
            .full_name = "Texkom TK13",
            .type_name = "TK13",
            .uid_offset = TEXKOM_UID_OFFSET,
            .uid_size = TEXKOM_UID_SIZE,
            .coding = TexkomCodingTk13,
            .crc = texkom_crc_tk13,
            .crc_mask = 0xFF,
        },
    [TexkomTypeTk15] =
        {
            .type_byte = TEXKOM_TYPE_BYTE_TK13,
            .full_name = "Texkom TK15",
            .type_name = "TK15",
            .uid_offset = TEXKOM_UID_OFFSET,
            .uid_size = TEXKOM_UID_SIZE,
            .coding = TexkomCodingTk13,
            .crc = texkom_crc_tk13,
            .crc_mask = 0xFF,
        },
    [TexkomTypeTk17] =
        {
            .type_byte = TEXKOM_TYPE_BYTE_TK17,
            .full_name = "Texkom TK17",
            .type_name = "TK17",
            .uid_offset = TEXKOM_UID_OFFSET,
            .uid_size = TEXKOM_UID_SIZE,
            .coding = TexkomCodingTk17,
            .crc = texkom_crc_tk17,
            .crc_mask = 0xFF,
        },
    [TexkomTypeMmbit] =
        {
            .type_byte = 0xFF,
            .full_name = "Texkom MMBIT",
            .type_name = "MMBIT",
            .uid_offset = TEXKOM_MMBIT_UID_OFFSET,
            .uid_size = TEXKOM_MMBIT_UID_SIZE,
            .coding = TexkomCodingTk13,
            .crc = texkom_crc_mmbit,
            .crc_mask = 0xF0,
        },
};

static TexkomType texkom_type_from_frame(const uint8_t* frame, TexkomCoding coding);

const NfcDeviceBase nfc_device_texkom = {
    .protocol_name = TEXKOM_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)texkom_alloc,
    .free = (NfcDeviceFree)texkom_free,
    .reset = (NfcDeviceReset)texkom_reset,
    .copy = (NfcDeviceCopy)texkom_copy,
    .verify = NULL,
    .load = (NfcDeviceLoad)texkom_load,
    .save = (NfcDeviceSave)texkom_save,
    .is_equal = (NfcDeviceEqual)texkom_is_equal,
    .get_name = (NfcDeviceGetName)texkom_get_device_name,
    .get_uid = (NfcDeviceGetUid)texkom_get_uid,
    .set_uid = (NfcDeviceSetUid)texkom_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)texkom_get_base_data,
};

static const TexkomFeatures* texkom_get_features(const TexkomData* data) {
    furi_check(data->type < TexkomTypeNum);

    return &texkom_features[data->type];
}

TexkomData* texkom_alloc(void) {
    TexkomData* data = malloc(sizeof(TexkomData));
    texkom_reset(data);

    return data;
}

void texkom_free(TexkomData* data) {
    furi_check(data);

    free(data);
}

void texkom_reset(TexkomData* data) {
    furi_check(data);

    memset(data, 0, sizeof(TexkomData));
    data->type = TexkomTypeUnknown;
}

void texkom_copy(TexkomData* data, const TexkomData* other) {
    furi_check(data);
    furi_check(other);

    *data = *other;
}

bool texkom_load(TexkomData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool parsed = false;
    FuriString* temp_str = furi_string_alloc();

    do {
        if(version < NFC_UNIFIED_FORMAT_VERSION) break;
        if(!flipper_format_read_string(ff, TEXKOM_TYPE_KEY, temp_str)) break;

        TexkomType type = TexkomTypeNum;
        for(size_t i = 0; i < TexkomTypeNum; i++) {
            if(furi_string_equal_str(temp_str, texkom_features[i].type_name)) {
                type = i;
                break;
            }
        }
        if(type == TexkomTypeNum || type == TexkomTypeUnknown) break;

        uint8_t frame[TEXKOM_FRAME_SIZE];
        if(!flipper_format_read_hex(ff, TEXKOM_FRAME_KEY, frame, TEXKOM_FRAME_SIZE)) break;

        // Everything downstream reads the variant off the type byte, so a hand-edited file must
        // not be able to put the stored type and the frame out of step. TK13 and TK15 are
        // indistinguishable here by design - same type byte, same coding, different timings.
        if(frame[0] != 0xFF || frame[1] != 0xFF) break;
        if(frame[2] != texkom_features[type].type_byte) break;
        if(type == TexkomTypeMmbit && frame[3] != 0xFF) break;

        memcpy(data->frame, frame, TEXKOM_FRAME_SIZE);
        data->type = type;
        parsed = true;
    } while(false);

    furi_string_free(temp_str);

    return parsed;
}

bool texkom_save(const TexkomData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    const TexkomFeatures* features = texkom_get_features(data);
    bool saved = false;

    do {
        if(!flipper_format_write_comment_cstr(ff, TEXKOM_PROTOCOL_NAME " specific data")) break;
        if(!flipper_format_write_string_cstr(ff, TEXKOM_TYPE_KEY, features->type_name)) break;
        if(!flipper_format_write_hex(ff, TEXKOM_FRAME_KEY, data->frame, TEXKOM_FRAME_SIZE)) break;

        saved = true;
    } while(false);

    return saved;
}

bool texkom_is_equal(const TexkomData* data, const TexkomData* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data, other, sizeof(TexkomData)) == 0; //-V1103
}

const char* texkom_get_device_name(const TexkomData* data, NfcDeviceNameType name_type) {
    furi_check(data);

    const TexkomFeatures* features = texkom_get_features(data);

    return (name_type == NfcDeviceNameTypeFull) ? features->full_name : features->type_name;
}

const uint8_t* texkom_get_uid(const TexkomData* data, size_t* uid_len) {
    furi_check(data);

    const TexkomFeatures* features = texkom_get_features(data);

    if(uid_len) {
        *uid_len = features->uid_size;
    }

    return &data->frame[features->uid_offset];
}

bool texkom_set_uid(TexkomData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    // The load path calls this before the frame is read, while the type is still Unknown: there
    // is no way to place a UID without knowing the variant, and the frame read next replaces it
    // anyway. Vet the length so a file with a nonsense UID still fails, and write nothing.
    if(data->type == TexkomTypeUnknown) {
        return uid_len == TEXKOM_UID_SIZE || uid_len == TEXKOM_MMBIT_UID_SIZE;
    }

    const TexkomFeatures* features = texkom_get_features(data);
    if(uid_len != features->uid_size) return false;

    memcpy(&data->frame[features->uid_offset], uid, uid_len);
    // A frame whose checksum disagrees with its UID is one a reader will not look at.
    texkom_update_crc(data);

    return true;
}

bool texkom_make_blank(TexkomData* data, TexkomType type) {
    furi_check(data);

    if(type == TexkomTypeUnknown || type >= TexkomTypeNum) return false;

    data->type = type;
    memset(data->frame, 0, TEXKOM_FRAME_SIZE);
    data->frame[0] = 0xFF;
    data->frame[1] = 0xFF;
    data->frame[2] = texkom_features[type].type_byte;
    // MMBIT spends the first UID byte on a second marker; everything else starts its UID there.
    if(type == TexkomTypeMmbit) {
        data->frame[3] = 0xFF;
    }
    texkom_update_crc(data);

    return true;
}

TexkomData* texkom_get_base_data(const TexkomData* data) {
    UNUSED(data);
    furi_crash("No base data");
}

/** CRC-8 over the 4 UID bytes: poly 0x31, zero init, MSB first. From PM3 TexcomTK13CRC. */
static uint8_t texkom_crc_tk13(const uint8_t* uid) {
    uint8_t crc = 0;

    for(size_t i = 0; i < TEXKOM_UID_SIZE; i++) {
        crc ^= uid[i];
        for(size_t bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? (0x31 ^ (crc << 1)) : (crc << 1);
        }
    }

    return crc;
}

/**
 * Dallas CRC-8 over the UID, the same one the iButton code uses.
 *
 * PM3 hashes a 7-byte buffer holding three leading zeros then the UID. Those zeros cannot change
 * the result - this CRC is reflected and starts at zero, so a zero byte leaves the register
 * alone - so the UID alone gives the same byte.
 */
static uint8_t texkom_crc_tk17(const uint8_t* uid) {
    return maxim_crc8(uid, TEXKOM_UID_SIZE, MAXIM_CRC8_INIT);
}

/** Nibble-wide XOR over the 3 UID bytes, inverted, aligned into the high nibble. */
static uint8_t texkom_crc_mmbit(const uint8_t* uid) {
    uint8_t crc = 0x0F;

    for(size_t i = 0; i < TEXKOM_MMBIT_UID_SIZE; i++) {
        crc ^= (uid[i] & 0x0F) ^ (uid[i] >> 4);
    }

    return (crc & 0x0F) << 4;
}

/**
 * @brief What the frame's last byte should hold, as a whole byte.
 *
 * The one place the masking rule lives: what the variant does not own stays as it is stored, so
 * MMBIT keeps its payload low nibble. Checking, reporting and writing the checksum are then all
 * the same computation.
 */
static bool texkom_crc_byte(const TexkomData* data, uint8_t* crc) {
    const TexkomFeatures* features = texkom_get_features(data);
    if(!features->crc) return false;

    const uint8_t stored = data->frame[TEXKOM_FRAME_SIZE - 1];

    *crc = (stored & ~features->crc_mask) | features->crc(&data->frame[features->uid_offset]);

    return true;
}

bool texkom_is_crc_valid(const TexkomData* data) {
    furi_check(data);

    uint8_t expected;

    return texkom_crc_byte(data, &expected) && expected == data->frame[TEXKOM_FRAME_SIZE - 1];
}

bool texkom_get_expected_crc(const TexkomData* data, uint8_t* crc) {
    furi_check(data);
    furi_check(crc);

    return texkom_crc_byte(data, crc);
}

void texkom_update_crc(TexkomData* data) {
    furi_check(data);

    uint8_t crc;
    if(texkom_crc_byte(data, &crc)) {
        data->frame[TEXKOM_FRAME_SIZE - 1] = crc;
    }
}

/**
 * @brief Average length of the long and the short gap in a capture.
 *
 * Split the gaps at the midpoint between the extremes and average each half, so the thresholds
 * follow however fast this particular tag happens to be running.
 */
static bool
    texkom_average_gaps(const uint8_t* intervals, size_t count, uint32_t* hi, uint32_t* low) {
    uint8_t max = 0;
    uint8_t min = UINT8_MAX;

    for(size_t i = 0; i < count; i++) {
        if(intervals[i] > max) max = intervals[i];
        if(intervals[i] < min) min = intervals[i];
    }
    if(max <= min) return false;

    const uint32_t middle = ((uint32_t)max + min) / 2;
    uint32_t hi_sum = 0, hi_count = 0, low_sum = 0, low_count = 0;

    for(size_t i = 0; i < count; i++) {
        if(intervals[i] > middle) {
            hi_sum += intervals[i];
            hi_count++;
        } else {
            low_sum += intervals[i];
            low_count++;
        }
    }
    if(hi_count == 0 || low_count == 0) return false;

    *hi = hi_sum / hi_count;
    *low = low_sum / low_count;

    return *hi > *low;
}

/**
 * Written without abs() so a tolerance wider than the length cannot wrap the comparison. PM3
 * subtracts in unsigned and does wrap, though never for a real tag's timings.
 */
static inline bool texkom_gap_matches(uint8_t interval, uint32_t length, uint32_t tolerance) {
    return (interval < length + tolerance) && (interval + tolerance > length);
}

/** Pack 64 one-per-byte bits, most significant first, into the frame. */
static void texkom_pack_frame(const uint8_t* bits, uint8_t* frame) {
    memset(frame, 0, TEXKOM_FRAME_SIZE);

    for(size_t i = 0; i < TEXKOM_FRAME_BITS; i++) {
        frame[i / 8] = (frame[i / 8] << 1) | (bits[i] & 1);
    }
}

static bool texkom_preamble_valid(const uint8_t* bits) {
    for(size_t i = 0; i < TEXKOM_PREAMBLE_BITS; i++) {
        if(!bits[i]) return false;
    }

    return true;
}

/**
 * @brief TK13/TK15/MMBIT coding: two gaps per data bit, long then short for a one.
 *
 * Each gap is classified against the averages, which only holds while the tag keeps the two
 * lengths well apart. Tags that do not are handled by texkom_decode_tk15().
 */
static bool texkom_decode_tk13(const uint8_t* intervals, size_t count, uint8_t* frame) {
    // Guaranteed by the dispatch in texkom_decode_intervals(); the bounds below are
    // belt-and-braces rather than live logic.
    furi_check(count == TEXKOM_TK13_INTERVALS);
    uint32_t hi = 0, low = 0;
    if(!texkom_average_gaps(intervals, count, &hi, &low)) return false;

    // Empirical, straight from PM3: wide enough for a tag running off-nominal, narrow enough to
    // keep the two gap lengths apart.
    const uint32_t tolerance = (hi - low) / 3 + 1;

    uint8_t raw[TEXKOM_TK13_RAW_BITS];
    size_t raw_count = 0;

    for(size_t i = 0; i < count && raw_count < TEXKOM_TK13_RAW_BITS; i++) {
        if(texkom_gap_matches(intervals[i], hi, tolerance)) {
            raw[raw_count++] = 1;
        } else if(texkom_gap_matches(intervals[i], low, tolerance)) {
            raw[raw_count++] = 0;
        } else {
            return false;
        }
    }

    // The gap after the frame's final impulse falls in the pause between repetitions and is never
    // measured, so the last bit arrives with only its first gap; its second is the complement.
    if(raw_count % 2 != 0 && raw_count < TEXKOM_TK13_RAW_BITS) {
        raw[raw_count] = !raw[raw_count - 1];
        raw_count++;
    }
    if(raw_count != TEXKOM_TK13_RAW_BITS) return false;

    uint8_t bits[TEXKOM_FRAME_BITS];
    for(size_t i = 0; i < TEXKOM_FRAME_BITS; i++) {
        if(raw[i * 2] == raw[i * 2 + 1]) return false;
        bits[i] = raw[i * 2];
    }

    if(!texkom_preamble_valid(bits)) return false;
    texkom_pack_frame(bits, frame);

    return true;
}

/**
 * @brief Same coding as TK13, decoded by comparing each pair of gaps to each other.
 *
 * Older tags run the two gap lengths close enough together that absolute thresholds stop
 * separating them, but within a single bit the long gap is still the longer of the two.
 */
static bool texkom_decode_tk15(const uint8_t* intervals, size_t count, uint8_t* frame) {
    // Guaranteed by the dispatch in texkom_decode_intervals(); the bounds below are
    // belt-and-braces rather than live logic.
    furi_check(count == TEXKOM_TK13_INTERVALS);
    uint8_t bits[TEXKOM_FRAME_BITS];
    size_t bit_count = 0;

    for(size_t i = 0; i + 1 < count && bit_count < TEXKOM_FRAME_BITS; i += 2) {
        if(intervals[i] == intervals[i + 1]) return false;
        bits[bit_count++] = intervals[i] > intervals[i + 1];
    }

    // As above, the last bit is one gap short. Decide it by which of the previous bit's two gaps
    // the orphan resembles more closely.
    if(count > 2 && count % 2 != 0 && bit_count < TEXKOM_FRAME_BITS) {
        const int last = intervals[count - 1];
        const bool previous_bit = intervals[count - 3] > intervals[count - 2];
        const bool same_as_previous = abs(last - (int)intervals[count - 3]) <
                                      abs(last - (int)intervals[count - 2]);

        bits[bit_count++] = previous_bit ^ (!same_as_previous);
    }

    if(bit_count != TEXKOM_FRAME_BITS) return false;
    if(!texkom_preamble_valid(bits)) return false;
    texkom_pack_frame(bits, frame);

    return true;
}

/**
 * @brief TK17 coding: a pair of gaps carries two bits in how it splits a fixed total.
 *
 * The pair always spans the same time; which of four bands the split falls into is the payload.
 */
static bool texkom_decode_tk17(const uint8_t* intervals, size_t count, uint8_t* frame) {
    // Guaranteed by the dispatch in texkom_decode_intervals(); the bounds below are
    // belt-and-braces rather than live logic.
    furi_check(count == TEXKOM_TK17_INTERVALS);
    uint8_t raw[TEXKOM_FRAME_BITS];
    size_t raw_count = 0;

    for(size_t i = 0; i + 1 < count && raw_count + 1 < TEXKOM_FRAME_BITS; i += 2) {
        const uint32_t total = (uint32_t)intervals[i] + intervals[i + 1];
        if(total == 0) return false;

        const uint32_t share = ((uint32_t)intervals[i + 1] * 100) / total;
        // A split this lopsided is none of the four, so the pair is noise. Bounds from PM3.
        if(share < 10 || share > 90) return false;

        // Bands, lowest share first: 00, 10, 01, 11.
        if(share < 30) {
            raw[raw_count++] = 0;
            raw[raw_count++] = 0;
        } else if(share < 50) {
            raw[raw_count++] = 1;
            raw[raw_count++] = 0;
        } else if(share < 70) {
            raw[raw_count++] = 0;
            raw[raw_count++] = 1;
        } else {
            raw[raw_count++] = 1;
            raw[raw_count++] = 1;
        }
    }

    if(raw_count != TEXKOM_FRAME_BITS) return false;

    // The tag sends each byte as four bit-pairs, least significant pair first; undo that.
    uint8_t bits[TEXKOM_FRAME_BITS];
    for(size_t byte = 0; byte < TEXKOM_FRAME_SIZE; byte++) {
        for(size_t pair = 0; pair < 4; pair++) {
            memcpy(&bits[byte * 8 + pair * 2], &raw[byte * 8 + (3 - pair) * 2], 2);
        }
    }

    if(!texkom_preamble_valid(bits)) return false;
    texkom_pack_frame(bits, frame);

    return true;
}

/**
 * @brief Variant a frame names, or Unknown if it names none that uses this coding.
 *
 * Requiring the two to agree is what lets the rest of the firmware read the variant off the type
 * byte alone, and it throws out noise that happened to survive the preamble check.
 */
static TexkomType texkom_type_from_frame(const uint8_t* frame, TexkomCoding coding) {
    TexkomType type;

    if(frame[2] == TEXKOM_TYPE_BYTE_TK17) {
        type = TexkomTypeTk17;
    } else if(frame[2] == TEXKOM_TYPE_BYTE_TK13) {
        type = TexkomTypeTk13;
    } else if(frame[2] == 0xFF && frame[3] == 0xFF) {
        type = TexkomTypeMmbit;
    } else {
        return TexkomTypeUnknown;
    }

    return (texkom_features[type].coding == coding) ? type : TexkomTypeUnknown;
}

bool texkom_decode_intervals(TexkomData* data, const uint8_t* intervals, size_t interval_count) {
    furi_check(data);
    furi_check(intervals);

    uint8_t frame[TEXKOM_FRAME_SIZE];
    TexkomType type = TexkomTypeUnknown;

    if(interval_count == TEXKOM_TK17_INTERVALS) {
        if(texkom_decode_tk17(intervals, interval_count, frame)) {
            type = texkom_type_from_frame(frame, TexkomCodingTk17);
        }
    } else if(interval_count == TEXKOM_TK13_INTERVALS) {
        if(texkom_decode_tk13(intervals, interval_count, frame)) {
            type = texkom_type_from_frame(frame, TexkomCodingTk13);
        } else if(texkom_decode_tk15(intervals, interval_count, frame)) {
            type = texkom_type_from_frame(frame, TexkomCodingTk13);
            // TK13 and TK15 are the same coding and differ only in how far apart the two gap
            // lengths run - which is exactly what made the thresholds above fail.
            if(type == TexkomTypeTk13) type = TexkomTypeTk15;
        }
    }

    if(type == TexkomTypeUnknown) return false;

    memcpy(data->frame, frame, TEXKOM_FRAME_SIZE);
    data->type = type;

    return true;
}
