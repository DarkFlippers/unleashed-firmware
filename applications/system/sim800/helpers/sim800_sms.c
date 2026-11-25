#include "sim800_sms.h"
#include "sim800_uart.h"

#define SIM800_CMGL_PREFIX "+CMGL:"
#define SIM800_CMGR_PREFIX "+CMGR:"

void sim800_inbox_init(Sim800Inbox* inbox) {
    furi_assert(inbox);
    for(size_t i = 0; i < SIM800_INBOX_MAX; i++) {
        inbox->items[i].index = 0;
        inbox->items[i].part_count = 0;
        inbox->items[i].last_part_full = false;
        inbox->items[i].status = furi_string_alloc();
        inbox->items[i].from = furi_string_alloc();
        inbox->items[i].date = furi_string_alloc();
        inbox->items[i].body = furi_string_alloc();
    }
    inbox->count = 0;
}

void sim800_inbox_free(Sim800Inbox* inbox) {
    furi_assert(inbox);
    for(size_t i = 0; i < SIM800_INBOX_MAX; i++) {
        furi_string_free(inbox->items[i].status);
        furi_string_free(inbox->items[i].from);
        furi_string_free(inbox->items[i].date);
        furi_string_free(inbox->items[i].body);
    }
    inbox->count = 0;
}

void sim800_inbox_reset(Sim800Inbox* inbox) {
    furi_assert(inbox);
    for(size_t i = 0; i < SIM800_INBOX_MAX; i++) {
        inbox->items[i].index = 0;
        inbox->items[i].part_count = 0;
        inbox->items[i].last_part_full = false;
        furi_string_reset(inbox->items[i].status);
        furi_string_reset(inbox->items[i].from);
        furi_string_reset(inbox->items[i].date);
        furi_string_reset(inbox->items[i].body);
    }
    inbox->count = 0;
}

void sim800_inbox_parse(Sim800Inbox* inbox, const FuriString* response) {
    furi_assert(inbox);
    furi_assert(response);

    sim800_inbox_reset(inbox);

    FuriString* line = furi_string_alloc();
    FuriString* field = furi_string_alloc();
    const char* cursor = furi_string_get_cstr(response);
    Sim800SmsEntry* current = NULL;

    while(*cursor != '\0') {
        const char* end = strchr(cursor, '\n');
        if(end) {
            furi_string_set_strn(line, cursor, (size_t)(end - cursor));
            cursor = end + 1;
        } else {
            furi_string_set_str(line, cursor);
            cursor += strlen(cursor);
        }

        if(furi_string_start_with_str(line, SIM800_CMGL_PREFIX)) {
            if(inbox->count >= SIM800_INBOX_MAX) break;
            current = &inbox->items[inbox->count];
            inbox->count++;

            const char* header = furi_string_get_cstr(line) + strlen(SIM800_CMGL_PREFIX);
            while(*header == ' ')
                header++;

            if(sim800_csv_field(header, 0, field)) {
                current->index = (uint32_t)strtoul(furi_string_get_cstr(field), NULL, 10);
                current->part_indices[0] = current->index;
                current->part_count = 1;
            }
            if(sim800_csv_field(header, 1, field)) furi_string_set(current->status, field);
            if(sim800_csv_field(header, 2, field)) furi_string_set(current->from, field);
            // Field 3 is the alphanumeric name, which is empty on most networks
            if(sim800_csv_field(header, 4, field)) furi_string_set(current->date, field);
            continue;
        }

        if(current == NULL) continue; // header lines before the first message

        if(furi_string_size(current->body) > 0) furi_string_cat_str(current->body, "\n");
        furi_string_cat(current->body, line);
    }

    furi_string_free(line);
    furi_string_free(field);
}

bool sim800_sms_parse_cmgr(Sim800SmsEntry* entry, uint32_t index, const FuriString* response) {
    furi_assert(entry);
    furi_assert(response);

    entry->index = index;
    entry->part_indices[0] = index;
    entry->part_count = 1;
    entry->last_part_full = false;
    furi_string_reset(entry->status);
    furi_string_reset(entry->from);
    furi_string_reset(entry->date);
    furi_string_reset(entry->body);

    FuriString* line = furi_string_alloc();
    FuriString* field = furi_string_alloc();
    const char* cursor = furi_string_get_cstr(response);
    bool found = false;

    while(*cursor != '\0') {
        const char* end = strchr(cursor, '\n');
        if(end) {
            furi_string_set_strn(line, cursor, (size_t)(end - cursor));
            cursor = end + 1;
        } else {
            furi_string_set_str(line, cursor);
            cursor += strlen(cursor);
        }

        if(!found && furi_string_start_with_str(line, SIM800_CMGR_PREFIX)) {
            found = true;
            const char* header = furi_string_get_cstr(line) + strlen(SIM800_CMGR_PREFIX);
            while(*header == ' ')
                header++;

            if(sim800_csv_field(header, 0, field)) furi_string_set(entry->status, field);
            if(sim800_csv_field(header, 1, field)) furi_string_set(entry->from, field);
            // Field 2 is the alphanumeric name, usually empty
            if(sim800_csv_field(header, 3, field)) furi_string_set(entry->date, field);
            continue;
        }

        if(!found) continue;

        if(furi_string_size(entry->body) > 0) furi_string_cat_str(entry->body, "\n");
        furi_string_cat(entry->body, line);
    }

    furi_string_free(line);
    furi_string_free(field);
    return found;
}

void sim800_sms_trim_preview(FuriString* body, size_t max_chars) {
    furi_assert(body);
    furi_string_replace_all_str(body, "\r", "");
    furi_string_replace_all_str(body, "\n", " ");

    const char* str = furi_string_get_cstr(body);
    const char* p = str;
    size_t chars = 0;

    while(*p != '\0' && chars < max_chars) {
        unsigned char c = (unsigned char)*p;
        if((c & 0x80) == 0) {
            p += 1;
        } else if((c & 0xE0) == 0xC0 && p[1] != '\0') {
            p += 2;
        } else if((c & 0xF0) == 0xE0 && p[1] && p[2]) {
            p += 3;
        } else if((c & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
            p += 4;
        } else {
            p += 1;
        }
        chars++;
    }

    if(*p != '\0') {
        furi_string_left(body, (size_t)(p - str));
    }
}

static bool sim800_parse_gsm_ts(const char* s, uint32_t* day_key, int32_t* sec_of_day) {
    // Format: YY/MM/DD,HH:MM:SS+ZZ
    if(!s || strlen(s) < 17) return false;
    if(s[2] != '/' || s[5] != '/' || s[8] != ',' || s[11] != ':' || s[14] != ':') return false;

    int yy = (s[0] - '0') * 10 + (s[1] - '0');
    int mm = (s[3] - '0') * 10 + (s[4] - '0');
    int dd = (s[6] - '0') * 10 + (s[7] - '0');
    int hh = (s[9] - '0') * 10 + (s[10] - '0');
    int mi = (s[12] - '0') * 10 + (s[13] - '0');
    int ss = (s[15] - '0') * 10 + (s[16] - '0');

    if(yy < 0 || yy > 99 || mm < 1 || mm > 12 || dd < 1 || dd > 31) return false;
    if(hh < 0 || hh > 23 || mi < 0 || mi > 59 || ss < 0 || ss > 59) return false;

    *day_key = (uint32_t)(yy * 10000 + mm * 100 + dd);
    *sec_of_day = hh * 3600 + mi * 60 + ss;
    return true;
}

bool sim800_sms_timestamps_close(const char* a, const char* b) {
    uint32_t day_a = 0, day_b = 0;
    int32_t sec_a = 0, sec_b = 0;
    if(!sim800_parse_gsm_ts(a, &day_a, &sec_a) || !sim800_parse_gsm_ts(b, &day_b, &sec_b)) {
        return false;
    }
    if(day_a != day_b) return false;
    int32_t diff = sec_a - sec_b;
    if(diff < 0) diff = -diff;
    return diff <= 30;
}

bool sim800_sms_part_looks_full(const char* body) {
    if(!body) return false;
    size_t chars = 0;
    bool non_ascii = false;
    const char* p = body;

    while(*p != '\0') {
        unsigned char c = (unsigned char)*p;
        if((c & 0x80) == 0) {
            p += 1;
        } else {
            non_ascii = true;
            if((c & 0xE0) == 0xC0 && p[1] != '\0') {
                p += 2;
            } else if((c & 0xF0) == 0xE0 && p[1] && p[2]) {
                p += 3;
            } else if((c & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
                p += 4;
            } else {
                p += 1;
            }
        }
        chars++;
    }

    // UCS2 multipart segment = 67 UTF-16 units; GSM-7 multipart segment = 153 septets
    return non_ascii ? (chars >= 62) : (chars >= 145);
}
